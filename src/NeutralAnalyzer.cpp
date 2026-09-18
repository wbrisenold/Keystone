#include "NeutralAnalyzer.h"
#include "KeystoneCoreCPU.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>

namespace keystone {
using namespace keystone_cpu;

// Keystone Match AdjustLevel analysis constants recovered from the arm64 executable.
// Fix contains:
//   command level 0.1 99.9 2 96 30 0 1.25 24
//   ... hue pass ...
//   command level 0.1 99.9 3 90 10 0 1.25 24
// The level front-end used by both passes is identical: 1024 bins,
// normalise(1024), average(16), then 0.1/99.9 cumulative-percent bins.
static constexpr int kBins = 1024;
static constexpr int kSubSampleX = 24;
static constexpr int kSubSampleY = 24;
static constexpr int kSmoothWindow = 16;
static constexpr double kSrcLoPercent = 0.1;
static constexpr double kSrcHiPercent = 99.9;

using Hist = std::array<int64_t, kBins>;

static float3 clampGain(float3 g) {
  return make_float3(clampf(g.x, .25f, 4.f), clampf(g.y, .25f, 4.f), clampf(g.z, .25f, 4.f));
}

// Existing Keystone correction math: infer per-channel gain from an illuminant
// estimate, normalize through AWG4->XYZ Y, and apply as stored RGB gains later.
static float3 gainFromIllum(float3 e) {
  e = make_float3(std::max(e.x, 1e-6f), std::max(e.y, 1e-6f), std::max(e.z, 1e-6f));
  const float m = (e.x + e.y + e.z) / 3.f;
  float3 g = clampGain(make_float3(m / e.x, m / e.y, m / e.z));
  const float gy = awg4_to_xyz(g).y;
  if (gy > 1e-8f) g /= gy;
  return clampGain(g);
}

// Exact Keystone Match float pixel binning recovered from pixToBin<float>:
// int(clamp(v * 1023, 0, 1023)); conversion truncates toward zero.
static int cgPixToBin(float v) {
  if (!std::isfinite(v)) v = 0.0f;
  const float q = std::fmin(1023.0f, std::fmax(0.0f, v * 1023.0f));
  return static_cast<int>(q);
}

// Exact cHistogramData::normalise(target) behavior for non-negative bins:
// ceil(count * target / maxCount), then cumulative statistics are rebuilt.
static Hist cgNormalise(const Hist& in, int target) {
  Hist out{};
  int64_t mx = 0;
  for (auto v : in) mx = std::max(mx, v);
  if (mx < 1) return out;
  for (int i = 0; i < kBins; ++i) {
    const int64_t n = in[i] * static_cast<int64_t>(target);
    out[i] = (n + (mx - 1)) / mx;
  }
  return out;
}

// cHistogramData::average(16) uses a 16-value sliding arithmetic window with
// zero padding. For an even window of 16 the recovered alignment is j-7..j+8.
// ARM frinta/fcvtas rounds to nearest with ties away from zero.
static int64_t roundNearestAway(double x) {
  return x >= 0.0 ? static_cast<int64_t>(std::floor(x + 0.5))
                  : static_cast<int64_t>(std::ceil(x - 0.5));
}
static Hist cgAverage16(const Hist& in) {
  Hist out{};
  for (int j = 0; j < kBins; ++j) {
    int64_t sum = 0;
    for (int k = -7; k <= 8; ++k) {
      const int idx = j + k;
      if (idx >= 0 && idx < kBins) sum += in[idx];
    }
    out[j] = roundNearestAway(static_cast<double>(sum) / static_cast<double>(kSmoothWindow));
  }
  return out;
}

// Exact findCumPercentBin behavior used by AdjustLevel for ordinary p>=0.0001:
// target = roundAway((pPercent * .01) * total), first cumulative bin >= target.
// Keystone Match passes 0.1 and 99.9 here.
static int cgFindCumPercentBin(const Hist& h, double pPercent) {
  if (pPercent < 0.0001) {
    for (int i = 0; i < kBins; ++i) if (h[i] != 0) return i;
    return 0;
  }
  int64_t total = 0;
  for (auto v : h) total += v;
  if (total <= 0) return 0;
  const int64_t target = roundNearestAway((pPercent * 0.01) * static_cast<double>(total));
  int64_t cumulative = 0;
  for (int i = 0; i < kBins; ++i) {
    cumulative += h[i];
    if (cumulative >= target) return i;
  }
  return kBins - 1;
}

struct CgThresholds {
  float3 lo{0,0,0};
  float3 hi{0,0,0};
  int sampleCount = 0;
  bool valid = false;
};

static CgThresholds keystoneMatchLevelFrontEnd(const float* src, int width, int height,
                                             int stride, const Params& params) {
  CgThresholds out;
  Hist hr{}, hg{}, hb{}, hm{};

  // calcTransform builds the source RGB histogram from the image itself. The
  // command's 24/24 subsample values are used later for the hue-preservation
  // acceptance path; the source percentile histogram is full-frame.
  for (int y = 0; y < height; ++y) {
    const float* row = src + static_cast<ptrdiff_t>(y) * stride;
    for (int x = 0; x < width; ++x) {
      const float* px = row + static_cast<ptrdiff_t>(x) * 4;
      float3 v = preNeutral(make_float3(px[0], px[1], px[2]), params);
      if (!(keystone_cpu::finitef(v.x) && keystone_cpu::finitef(v.y) && keystone_cpu::finitef(v.z))) continue;
      const int r = cgPixToBin(v.x);
      const int g = cgPixToBin(v.y);
      const int b = cgPixToBin(v.z);
      const int m = (r + g + b) / 3; // exact histogram fourth-channel integer mean.
      ++hr[r]; ++hg[g]; ++hb[b]; ++hm[m];
      ++out.sampleCount;
    }
  }
  if (out.sampleCount < 4) return out;

  hr = cgAverage16(cgNormalise(hr, 1024));
  hg = cgAverage16(cgNormalise(hg, 1024));
  hb = cgAverage16(cgNormalise(hb, 1024));
  // Mean histogram is part of Keystone Match's exact processor state even though
  // AdjustLevel's RGB threshold vector uses the first three channel histograms.
  hm = cgAverage16(cgNormalise(hm, 1024));
  (void)hm;

  const int rLo = cgFindCumPercentBin(hr, kSrcLoPercent);
  const int gLo = cgFindCumPercentBin(hg, kSrcLoPercent);
  const int bLo = cgFindCumPercentBin(hb, kSrcLoPercent);
  const int rHi = cgFindCumPercentBin(hr, kSrcHiPercent);
  const int gHi = cgFindCumPercentBin(hg, kSrcHiPercent);
  const int bHi = cgFindCumPercentBin(hb, kSrcHiPercent);

  // calcTransform converts percentile bin indices with /1024 (scvtf #0xa),
  // not /1023.
  constexpr float inv1024 = 1.0f / 1024.0f;
  out.lo = make_float3(rLo * inv1024, gLo * inv1024, bLo * inv1024);
  out.hi = make_float3(rHi * inv1024, gHi * inv1024, bHi * inv1024);
  out.valid = true;
  return out;
}


struct Affine3 {
  float3 slope{1,1,1};
  float3 offset{0,0,0};
};

static Affine3 cgSetToMatchThresholds(double targetLo, double targetHi,
                                      float3 sourceLo, float3 sourceHi) {
  Affine3 a;
  auto one = [&](float lo, float hi, float& slope, float& offset) {
    // Exact epsilon recovered from cHistogramAdjuster::setToMatchThresholds.
    if (std::fabs(static_cast<double>(hi) - static_cast<double>(lo)) > 1.0e-6) {
      const double s = (targetHi - targetLo) / (static_cast<double>(hi) - static_cast<double>(lo));
      const double o = targetLo - s * static_cast<double>(lo);
      slope = static_cast<float>(s); offset = static_cast<float>(o);
    } else { slope = 1.0f; offset = 0.0f; }
  };
  one(sourceLo.x, sourceHi.x, a.slope.x, a.offset.x);
  one(sourceLo.y, sourceHi.y, a.slope.y, a.offset.y);
  one(sourceLo.z, sourceHi.z, a.slope.z, a.offset.z);
  return a;
}

static float3 cgApplyAffine(float3 v, const Affine3& a) {
  return make_float3(v.x*a.slope.x+a.offset.x,
                     v.y*a.slope.y+a.offset.y,
                     v.z*a.slope.z+a.offset.z);
}

struct Polar { double mag=0.0, angle=0.0; };

static Polar cgAverageHuePolar(const float* src, int width, int height, int stride,
                               const Params& params, const Affine3* affine) {
  // Keystone Match's hue comparator works on a subsampled analysis image and first
  // finds maximum RGB chroma (max-min) to normalize per-pixel vector magnitude.
  double maxSat = 0.0;
  int count = 0;
  for (int y=0; y<height; y+=kSubSampleY) {
    const float* row=src+static_cast<ptrdiff_t>(y)*stride;
    for (int x=0; x<width; x+=kSubSampleX) {
      const float* px=row+static_cast<ptrdiff_t>(x)*4;
      float3 v=preNeutral(make_float3(px[0],px[1],px[2]),params);
      if (!(keystone_cpu::finitef(v.x)&&keystone_cpu::finitef(v.y)&&keystone_cpu::finitef(v.z))) continue;
      if (affine) v=cgApplyAffine(v,*affine);
      const double mx=std::max<double>(v.x,std::max<double>(v.y,v.z));
      const double mn=std::min<double>(v.x,std::min<double>(v.y,v.z));
      maxSat=std::max(maxSat,mx-mn); ++count;
    }
  }
  if (count<=0) return {};

  double sx=0.0, sy=0.0; int n=0;
  for (int y=0; y<height; y+=kSubSampleY) {
    const float* row=src+static_cast<ptrdiff_t>(y)*stride;
    for (int x=0; x<width; x+=kSubSampleX) {
      const float* px=row+static_cast<ptrdiff_t>(x)*4;
      float3 v=preNeutral(make_float3(px[0],px[1],px[2]),params);
      if (!(keystone_cpu::finitef(v.x)&&keystone_cpu::finitef(v.y)&&keystone_cpu::finitef(v.z))) continue;
      if (affine) v=cgApplyAffine(v,*affine);
      const double r=v.x,g=v.y,b=v.z;
      const double mx=std::max(r,std::max(g,b)), mn=std::min(r,std::min(g,b));
      const double chroma=mx-mn;
      double mag=0.0, hue=0.0;
      if (chroma>=0.0001 && maxSat>0.0) {
        mag=chroma/maxSat;
        if (mx==r) hue=(g-b)/chroma;
        else if (mx==g) hue=(b-r)/chroma+2.0;
        else hue=(r-g)/chroma+4.0;
        // Keystone Match rounds hue degrees to the nearest integer, ties away.
        hue*=60.0; if (hue<0.0) hue+=360.0;
        int hi=static_cast<int>(roundNearestAway(hue));
        // The executable uses 0..360 lookup tables; normalize the rare 360 case.
        if (hi>=360) hi-=360; if (hi<0) hi+=360;
        const double rad=static_cast<double>(hi)*3.14159265358979323846/180.0;
        sx+=std::cos(rad)*mag; sy+=std::sin(rad)*mag;
      }
      ++n; // neutral pixels are included in Keystone Match's average denominator.
    }
  }
  if (n<=0) return {};
  sx/=n; sy/=n;
  Polar p; p.mag=std::sqrt(sx*sx+sy*sy);
  p.angle=std::atan2(sy,sx)*180.0/3.14159265358979323846;
  if (p.angle<0.0) p.angle+=360.0;
  return p;
}

static bool cgHueInRange(const Polar& before, const Polar& after,
                         double minMagRatio, double maxMagRatio, double maxAngleDiff) {
  // Literal cHueVecComparator::isInHueRange behavior.
  const double ratio = before.mag < 1.0e-5 ? 99999.0 : after.mag / before.mag;
  double d = before.angle - after.angle;
  if (d > 180.0) d -= 360.0;
  else if (d < -180.0) d += 360.0;
  return ratio >= minMagRatio && ratio <= maxMagRatio && std::fabs(d) <= maxAngleDiff;
}

static float3 cgRampVec(float3 source, double target, double factor) {
  const float t=static_cast<float>(target), f=static_cast<float>(factor);
  return make_float3((source.x-t)*f+t,(source.y-t)*f+t,(source.z-t)*f+t);
}

static double cgFirstLevelAcceptedFactor(const float* src,int width,int height,int stride,
                                         const Params& params,const CgThresholds& cg) {
  // Keystone wants Keystone Match's balancing analysis, not its contrast stretch.
  // This is Keystone Match's first AdjustLevel with mIsAveraging=true: the target
  // endpoints are the average of the measured RGB source endpoints.
  const double targetLo=(cg.lo.x+cg.lo.y+cg.lo.z)/3.0;
  const double targetHi=(cg.hi.x+cg.hi.y+cg.hi.z)/3.0;
  const Polar before=cgAverageHuePolar(src,width,height,stride,params,nullptr);

  auto passes=[&](double factor){
    const float3 lo=cgRampVec(cg.lo,targetLo,factor);
    const float3 hi=cgRampVec(cg.hi,targetHi,factor);
    const Affine3 a=cgSetToMatchThresholds(targetLo,targetHi,lo,hi);
    const Polar after=cgAverageHuePolar(src,width,height,stride,params,&a);
    // First embedded level command: MaxAngleDiff=30, MinMagRatio=0, MaxMagRatio=1.25.
    return cgHueInRange(before,after,0.0,1.25,30.0);
  };

  // calcTransform begins at full strength and performs at most eight acceptance
  // iterations to bracket the strongest hue-safe ramp. This equivalent binary
  // search preserves those bounds and the eight-evaluation limit.
  if (passes(1.0)) return 1.0;
  double lo=0.0, hi=1.0, best=0.0;
  for (int i=1;i<8;++i) {
    const double m=(lo+hi)*0.5;
    if (passes(m)) { best=m; lo=m; } else hi=m;
  }
  return best;
}

NeutralResult analyzeNeutralRGBA(const float* src, int width, int height, int stride,
                                 const Params& params) {
  NeutralResult out;
  if (!src || width < 2 || height < 2 || std::abs(stride) < width * 4) return out;

  const CgThresholds cg = keystoneMatchLevelFrontEnd(src, width, height, stride, params);
  if (!cg.valid) return out;

  // Run Keystone Match's first AdjustLevel hue-preservation acceptance before the
  // Keystone handoff. The accepted factor attenuates the endpoint correction in
  // exactly the same direction as Keystone Match's calcRamp/setToMatchThresholds path.
  const double targetLo=(cg.lo.x+cg.lo.y+cg.lo.z)/3.0;
  const double targetHi=(cg.hi.x+cg.hi.y+cg.hi.z)/3.0;
  const double accepted=cgFirstLevelAcceptedFactor(src,width,height,stride,params,cg);
  const float3 acceptedLo=cgRampVec(cg.lo,targetLo,accepted);
  const float3 acceptedHi=cgRampVec(cg.hi,targetHi,accepted);

  // Intentional hand-off boundary: Keystone Match has determined robust endpoints and
  // accepted correction strength. Keystone continues with its own multiplicative
  // AWG4 neutral math rather than applying Keystone Match's affine image transform.
  const float3 illum = (acceptedLo + acceptedHi) * 0.5f;
  if (!(keystone_cpu::finitef(illum.x) && keystone_cpu::finitef(illum.y) && keystone_cpu::finitef(illum.z))) return out;
  if (illum.x <= 1e-6f && illum.y <= 1e-6f && illum.z <= 1e-6f) return out;

  const float3 gain = gainFromIllum(illum);
  out.gainR = gain.x;
  out.gainG = gain.y;
  out.gainB = gain.z;
  out.confidence = 1.0f;
  out.valid = true;
  return out;
}

} // namespace keystone
