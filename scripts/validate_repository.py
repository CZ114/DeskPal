#!/usr/bin/env python3
"""Validate packaging and provenance. Does not build or run the firmware."""
from pathlib import Path
import hashlib
import json
import re
import sys

ROOT = Path(__file__).resolve().parents[1]
errors = []
required = ["LICENSE", "NOTICE", "README.md", "README.zh-CN.md", "THIRD_PARTY_NOTICES.md",
            "firmware/DeskPal/DeskPal.ino", "firmware/DeskPal/config.example.h",
            "docs/SETUP.md", "docs/ARCHITECTURE.md", "docs/DEMO.md",
            "docs/MATERIALS.md", "docs/KNOWN_LIMITATIONS.md", "docs/vendor-manifest.json"]
for name in required:
    if not (ROOT / name).is_file(): errors.append(f"Missing {name}")

manifest = json.loads((ROOT / "docs/vendor-manifest.json").read_text(encoding="utf8"))
for entry in manifest:
    file = (ROOT / entry["path"]).resolve()
    if not file.is_relative_to(ROOT) or not file.is_file():
        errors.append(f"Invalid or missing vendored file: {entry['path']}")
    elif hashlib.sha256(file.read_bytes()).hexdigest() != entry["sha256"]:
        errors.append(f"Modified vendored file: {entry['path']}")

sketch = (ROOT / "firmware/DeskPal/DeskPal.ino").read_text(encoding="utf8")
for name in ["ssid", "password", "mqtt_server", "ID"]:
    if re.search(rf'const\s+char\s*\*\s*{name}\s*=\s*"', sketch):
        errors.append(f"Inline private configuration: {name}")
if '#include "config.h"' not in sketch: errors.append("Missing private configuration include")
ignore = (ROOT / ".gitignore").read_text(encoding="utf8")
if "firmware/DeskPal/config.h" not in ignore: errors.append("config.h is not ignored")

model = ROOT / "libraries/PoseDetection_inferencing/src/model-parameters"
metadata = (model / "model_metadata.h").read_text(encoding="utf8")
variables = (model / "model_variables.h").read_text(encoding="utf8")
for key, value in {"EI_CLASSIFIER_RAW_SAMPLE_COUNT": "125", "EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME": "3",
                   "EI_CLASSIFIER_NN_INPUT_FRAME_SIZE": "78", "EI_CLASSIFIER_INTERVAL_MS": "16"}.items():
    if not re.search(rf"#define\s+{key}\s+{re.escape(value)}\s*(?:\n|$)", metadata):
        errors.append(f"Documented model metadata mismatch: {key}")
if '{ "idle", "rise", "wave" }' not in variables: errors.append("Unexpected model class order")
payload = json.loads((ROOT / "examples/weather-response.json").read_text(encoding="utf8"))
if set(payload) != {"city", "weather"} or not all(isinstance(v, str) and v for v in payload.values()):
    errors.append("Invalid example weather response")

for md in [ROOT / "README.md", ROOT / "README.zh-CN.md", ROOT / "THIRD_PARTY_NOTICES.md", *sorted((ROOT / "docs").glob("*.md"))]:
    for target in re.findall(r"\]\(([^)]+)\)", md.read_text(encoding="utf8")):
        if target.startswith(("http:", "https:", "#", "mailto:")): continue
        target = target.split("#", 1)[0]
        resolved = (md.parent / target).resolve()
        if not resolved.is_relative_to(ROOT) or not resolved.exists():
            errors.append(f"Broken or nonportable link in {md.relative_to(ROOT)}: {target}")

if errors:
    print("FAIL\n" + "\n".join(errors))
    sys.exit(1)
print(f"PASS: {len(manifest)} vendored file hashes, required files, configuration isolation, model metadata and local documentation links.")
print("Firmware compilation, hardware behavior and model accuracy were NOT tested.")
