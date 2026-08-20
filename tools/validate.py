#!/usr/bin/env python3
from pathlib import Path
import re
import sys

EXPECTED_TRANSFORM = "__DEVICE__ float3 transform(int p_Width,int p_Height,int p_X,int p_Y,float p_R,float p_G,float p_B){"

def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//.*", "", text)
    return text

def main():
    path = Path(sys.argv[1] if len(sys.argv) > 1 else "Keystone.dctl")
    if not path.exists():
        print(f"FAIL: missing {path}")
        return 1

    text = path.read_text()
    code = strip_comments(text)
    errors = []

    if code.count("{") != code.count("}"):
        errors.append(f"unbalanced braces: {code.count('{')} / {code.count('}')}")
    if code.count("(") != code.count(")"):
        errors.append(f"unbalanced parentheses: {code.count('(')} / {code.count(')')}")

    transform_count = text.count("__DEVICE__ float3 transform(")
    if transform_count != 1:
        errors.append(f"expected exactly one float3 transform(), found {transform_count}")

    if EXPECTED_TRANSFORM not in text:
        errors.append("transform signature does not match Keystone's required one-line signature")

    ui_lines = [line for line in text.splitlines() if line.startswith("DEFINE_UI_PARAMS")]
    for line in ui_lines:
        if line.count("(") != line.count(")"):
            errors.append(f"multiline/malformed UI macro: {line}")
        m = re.match(r"DEFINE_UI_PARAMS\(([^,]+),\s*(.*?),\s*(DCTLUI_[A-Z_]+),", line)
        if not m:
            errors.append(f"could not parse UI macro: {line}")
            continue
        label = m.group(2)
        if "(" in label or ")" in label:
            errors.append(f"parser-sensitive parentheses in UI label: {label}")

    if re.search(r"\bwhile\s*\(", text):
        errors.append("while loop found; Keystone avoids while for Resolve/Metal parser conservatism")

    if re.search(r"(?<![A-Za-z0-9_])PI(?![A-Za-z0-9_])", code):
        errors.append("bare PI identifier found; use a literal or defined Keystone constant")

    functions = re.findall(
        r"__DEVICE__\s+(?:float3|float2|float|int|bool)\s+([A-Za-z_]\w*)\s*\(",
        text
    )
    for fn in functions:
        if fn == "transform":
            continue
        uses = len(re.findall(r"\b" + re.escape(fn) + r"\s*\(", text))
        if uses == 1:
            errors.append(f"unused __DEVICE__ helper: {fn}")

    if errors:
        print("Keystone DCTL validation: FAIL")
        for error in errors:
            print(f" - {error}")
        return 1

    print("Keystone DCTL validation: PASS")
    print(f" UI controls: {len(ui_lines)}")
    print(" NOTE: static validation does not replace compiling the DCTL in DaVinci Resolve.")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
