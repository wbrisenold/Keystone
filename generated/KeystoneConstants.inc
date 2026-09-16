// Keystone v4.5.11 — LogC4 grading + original Keystone Output LogC4 output + Keystone Interlayer + Keystone color tools
// Color Sep removed.
// Adds Color / Interlayer using the recovered Keystone 5207 strength-dependent IIE polynomial term.
// Adds recovered Keystone 2.1.6 Density, Split Saturation, and Split Toning processing math.
// Keystone's 65^3 LUT export path, cube-render code, overlays, watermark code, and UI graph code are NOT included.
// No Keystone LUTs, stock D-LogE tables, halation, or grain are embedded.
// All Keystone grading stages remain in the established ARRI Wide Gamut 4 / LogC4 pipeline.
// Final rendering uses the original user-supplied 33^3 Keystone Output LogC4 -> Rec.709/BT.1886 LUT.
// The original Keystone Output lattice values are embedded unchanged; no hybrid output is used.
// Keystone evaluates the lattice with its own trilinear sampler rather than Resolve's project LUT interpolation.
// Out-of-domain LogC4 channels use boundary-cell linear extension to avoid hard cube-edge clipping.
// Exact recovered Keystone Bleach Bypass runs only after the Keystone Output Rec.709 render.
// No skin detection/protection is applied to Color / Bleach.
// SPDX-License-Identifier: GPL-3.0-only
//
// DaVinci Resolve DCTL for ARRI Wide Gamut 4 / LogC4.
//
// Source-anchored photochemical core. Creative color presets preserve the legacy Keystone v2.7 AWG3 density-tone behavior
// through an explicit AWG4 -> XYZ -> AWG3 -> legacy tone -> XYZ -> AWG4 bridge.
//
// Attribution, upstream links, licenses, and implementation provenance are
// documented outside the DCTL in README.md, SOURCE_AUDIT.md, and
// THIRD_PARTY_NOTICES.md.

// ---- ARRI Wide Gamut 4 -> XYZ D65 (ARRI LogC4 specification) ----
#define AWG4_TO_XYZ_C0 make_float3(0.704858320407232f,0.254524176404027f,0.000000000000000f)
#define AWG4_TO_XYZ_C1 make_float3(0.129760295170463f,0.781477732712002f,0.000000000000000f)
#define AWG4_TO_XYZ_C2 make_float3(0.115837311473977f,-0.036001909116029f,1.089057750759878f)

// ---- D65 reference white ----
// Retained for Keystone Output Creative White and neutral-axis safety fallbacks.
#define WHITE_D65 make_float3(0.9504559271f,1.0f,1.0890577508f)


// ---- Keystone Output v1.1.0 Creative White CAT02 matrices (Jed Smith, GPLv3) ----
// Source: uploaded Keystone Output.dctl / open-display-transform.
// These are the exact D65 -> target-white matrices used by Keystone Output's Creative White selector.
#define ODRT_D65_D93_R0 make_float3(0.95703423023223877f,-0.0247171502560377121f,0.0624028593301773071f)
#define ODRT_D65_D93_R1 make_float3(-0.0179296955466270447f,0.990019857883453369f,0.0248119533061981201f)
#define ODRT_D65_D93_R2 make_float3(0.00127589143812656403f,0.00427919067442417058f,1.29345715045928955f)

#define ODRT_D65_D75_R0 make_float3(0.981001079082489014f,-0.0116619253531098366f,0.0265614092350006104f)
#define ODRT_D65_D75_R1 make_float3(-0.00843488052487373352f,0.996506094932556152f,0.0105696544051170349f)
#define ODRT_D65_D75_R2 make_float3(0.000552809564396739006f,0.00179840810596942902f,1.12374722957611084f)

#define ODRT_D65_D60_R0 make_float3(1.01182246208190918f,0.00778879318386316299f,-0.0157783031463623047f)
#define ODRT_D65_D60_R1 make_float3(0.00561682833358645439f,1.00150644779205322f,-0.00628517568111419678f)
#define ODRT_D65_D60_R2 make_float3(-0.000335735734552145004f,-0.0010509500280022619f,0.927366673946380615f)

#define ODRT_D65_D55_R0 make_float3(1.02585089206695557f,0.0179439820349216461f,-0.0332137793302536011f)
#define ODRT_D65_D55_R1 make_float3(0.0129133854061365128f,1.00214779376983643f,-0.0132421031594276428f)
#define ODRT_D65_D55_R2 make_float3(-0.000719940289855003032f,-0.00218106806278228803f,0.84868013858795166f)

#define ODRT_D65_D50_R0 make_float3(1.04257404804229736f,0.03089117631316185f,-0.052812620997428894f)
#define ODRT_D65_D50_R1 make_float3(0.0221935361623764038f,1.00185668468475342f,-0.0210737623274326324f)
#define ODRT_D65_D50_R2 make_float3(-0.00116488314233720303f,-0.00342052709311246915f,0.761789083480834961f)

// ---- XYZ D65 -> ARRI Wide Gamut 4 ----
#define XYZ_TO_AWG4_C0 make_float3(1.509215472242109f,-0.491545451660619f,0.000000000000000f)
#define XYZ_TO_AWG4_C1 make_float3(-0.250597345204380f,1.361245545929351f,0.000000000000000f)
#define XYZ_TO_AWG4_C2 make_float3(-0.168811475294073f,0.097282942013729f,0.918224951158247f)

// ---- Rec.709 / D65 bridge for the recovered Keystone display-referred Basic block ----
// Row-form matrices. Keystone remains LogC4/AWG4 overall; only the recovered
// Fade temporarily works in Rec.709 gamma 2.4. Color Bleach is removed.
#define XYZ_TO_REC709_R0 make_float3(3.2409699419045226f,-1.5373831775700940f,-0.4986107602930034f)
#define XYZ_TO_REC709_R1 make_float3(-0.9692436362808796f,1.8759675015077202f,0.0415550574071756f)
#define XYZ_TO_REC709_R2 make_float3(0.0556300796969937f,-0.2039769588889765f,1.0569715142428786f)
#define REC709_TO_XYZ_R0 make_float3(0.4123907992659595f,0.3575843393838779f,0.1804807884018343f)
#define REC709_TO_XYZ_R1 make_float3(0.2126390058715104f,0.7151686787677559f,0.0721923153607337f)
#define REC709_TO_XYZ_R2 make_float3(0.0193308187155918f,0.1191947797946260f,0.9505321522496607f)

// Recovered Keystone skin-selection defaults.
#define KS_SKIN_SKIN_TARGET_HUE_DEG 20.0f
#define KS_SKIN_SKIN_RANGE_DEG 40.0f
#define KS_SKIN_SKIN_LIGHTNESS_CENTER 0.5f
#define KS_SKIN_SKIN_LIGHTNESS_WIDTH 0.4f

// ---- Legacy Keystone AWG3 bridge for color-tone presets only ----
// These are the exact AWG3/XYZ matrices used by the older Keystone density-tone engine.
#define AWG3_TO_XYZ_C0 make_float3(0.638008f,0.291954f,0.002798f)
#define AWG3_TO_XYZ_C1 make_float3(0.214704f,0.823841f,-0.067034f)
#define AWG3_TO_XYZ_C2 make_float3(0.097744f,-0.115795f,1.153294f)
#define XYZ_TO_AWG3_C0 make_float3(1.789064538025f,-0.639848741771f,-0.041530974004f)
#define XYZ_TO_AWG3_C1 make_float3(-0.482533816466f,1.396400099532f,0.082334958727f)
#define XYZ_TO_AWG3_C2 make_float3(-0.200075026387f,0.194432230586f,0.878868163772f)

// ---- OKLab direct XYZ D65 matrices ----
#define X65_TO_OKLMS_C0 make_float3(0.8190224432f,0.0329836672f,0.0481771996f)
#define X65_TO_OKLMS_C1 make_float3(0.3619062563f,0.9292868469f,0.2642395197f)
#define X65_TO_OKLMS_C2 make_float3(-0.1288737826f,0.0361446682f,0.6335478285f)
#define OKLMS3_TO_X65_C0 make_float3(1.2268798734f,-0.0405757646f,-0.0763729497f)
#define OKLMS3_TO_X65_C1 make_float3(-0.5578150037f,1.1122868294f,-0.4214933239f)
#define OKLMS3_TO_X65_C2 make_float3(0.2813910502f,-0.0717110667f,1.5869240244f)
#define O_C0 make_float3(0.2104542553f,1.9779984951f,0.0259040371f)
#define O_C1 make_float3(0.793617785f,-2.428592205f,0.7827717662f)
#define O_C2 make_float3(-0.0040720468f,0.4505937099f,-0.808675766f)
#define P_C0 make_float3(1.0f,1.0f,1.0f)
#define P_C1 make_float3(0.3963377774f,-0.1055613458f,-0.0894841775f)
#define P_C2 make_float3(0.2158037573f,-0.0638541728f,-1.291485548f)

// AP1 wrapper used only by optional ACES 1.3 Reference Gamut Compression.
#define X65_TO_AP1_C0 make_float3(1.645137477880f,-0.663190970565f,0.011827536679f)
#define X65_TO_AP1_C1 make_float3(-0.305728269983f,1.605239925856f,-0.011607564142f)
#define X65_TO_AP1_C2 make_float3(-0.236812415835f,0.023041811312f,0.918561032294f)
#define AP1_TO_X65_C0 make_float3(0.657631916906f,0.271767176760f,-0.005033538330f)
#define AP1_TO_X65_C1 make_float3(0.126453267991f,0.675103815523f,0.006902836343f)
#define AP1_TO_X65_C2 make_float3(0.166370741628f,0.053129008203f,1.087188452940f)

// ---- ARRI LogC4 curve + Chroma-style opponent WB constants ----
// LogC4 constants are evaluated from the equations in the ARRI LogC4 specification.
#define LOGC4_A 2231.826309067688271f
#define LOGC4_B 0.907135874877810f
#define LOGC4_C 0.092864125122190f
#define LOGC4_S 0.113597208610589f
#define LOGC4_T -0.018056996119911f

// Temp/Tint are independent, orthogonal opponent axes in log-gain space.
// Temp: warm/cool = +R / -B. Tint: magenta/green = +R+B / -G.
// The vectors are unit-length and orthogonal to the neutral [1,1,1] axis,
// so the control starts chromatic rather than adding a neutral exposure component.
#define WB_TEMP_AXIS_R 0.7071067811865475f
#define WB_TEMP_AXIS_G 0.0000000000000000f
#define WB_TEMP_AXIS_B -0.7071067811865475f
#define WB_TINT_AXIS_R 0.4082482904638630f
#define WB_TINT_AXIS_G -0.8164965809277260f
#define WB_TINT_AXIS_B 0.4082482904638630f
#define WB_TEMP_MAX_STOPS 1.0000000000000000f
#define WB_TINT_MAX_STOPS 0.7000000000000000f


// ---- Keystone source-derived working constants ----
#define KS_LOG10_2 0.301029996f

// ---- Keystone 5207 Interlayer / Interimage Effect ----
// Exact recovered 3x10 polynomial coefficients for Kodak 5207. Keystone scales
// only the quadratic terms (columns 3..8) with its Interlayer Effect strength.
// Keystone intentionally imports only that strength-dependent term: the fixed
// stock transform and measured D-LogE tables are NOT embedded.
#define GEN_5207_IIE_R0_0  1.018700004f
#define GEN_5207_IIE_R0_1  0.004300000f
#define GEN_5207_IIE_R0_2 -0.023000000f
#define GEN_5207_IIE_R0_3  0.152600005f
#define GEN_5207_IIE_R0_4  0.000000000f
#define GEN_5207_IIE_R0_5  0.000000000f
#define GEN_5207_IIE_R0_6 -0.124399997f
#define GEN_5207_IIE_R0_7 -0.028300000f
#define GEN_5207_IIE_R0_8  0.000000000f
#define GEN_5207_IIE_R0_9  0.000000000f
#define GEN_5207_IIE_R1_0  0.083999999f
#define GEN_5207_IIE_R1_1  0.848200023f
#define GEN_5207_IIE_R1_2  0.067699999f
#define GEN_5207_IIE_R1_3  0.000000000f
#define GEN_5207_IIE_R1_4  0.363099992f
#define GEN_5207_IIE_R1_5  0.000000000f
#define GEN_5207_IIE_R1_6 -0.284000009f
#define GEN_5207_IIE_R1_7  0.000000000f
#define GEN_5207_IIE_R1_8 -0.079000004f
#define GEN_5207_IIE_R1_9  0.000000000f
#define GEN_5207_IIE_R2_0 -0.037900001f
#define GEN_5207_IIE_R2_1  0.027400000f
#define GEN_5207_IIE_R2_2  1.010499954f
#define GEN_5207_IIE_R2_3  0.000000000f
#define GEN_5207_IIE_R2_4  0.000000000f
#define GEN_5207_IIE_R2_5  0.264299989f
#define GEN_5207_IIE_R2_6  0.000000000f
#define GEN_5207_IIE_R2_7 -0.086800002f
#define GEN_5207_IIE_R2_8 -0.177499995f
#define GEN_5207_IIE_R2_9  0.000000000f
#define KS_TAU 6.2831853071795864769f
#define KS_GRAY_D 0.744727f
#define KS_INTERNAL_CURVE_AMOUNT 1.0f
#define KS_INTERNAL_CREATIVE_WHITE_LIMIT 0.25f

// ---- Original Keystone Output LogC4 output — embedded exact 33^3 table ----
// Source: user-supplied Keystone_Output_LogC4_to_Rec709.cube.
// Original metadata: Input Color Space LogC4; Output BT.1886; Resolution 33x33x33.
// Exact source lattice values are embedded unchanged. No Keystone Output-only outputization,
// refit, resampling, or newly authored LUT data is used here.
// Keystone fetches exact lattice nodes and performs its own trilinear interpolation.
// Out-of-domain LogC4 channels use the same boundary-cell linear extension strategy
// already used by the v4.5.6 output sampler; this is sampler behavior, not LUT data.
