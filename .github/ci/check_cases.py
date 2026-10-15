#!/usr/bin/env python3
"""Validate the allowlisted HGGC samples used by GitHub CI."""

import json
import re
import sys
from pathlib import Path, PurePosixPath

CI_DIR = Path(__file__).resolve().parent
REPO_ROOT = CI_DIR.parents[1]
CONFIG_PATH = CI_DIR / "cases.json"
SAFE_NAME = re.compile(r"^[A-Za-z0-9_.-]+$")
TARGET_PATTERN = r"\badd_executable\s*\(\s*{}(?:\s|\))"


def fail(message):
    print(f"error: {message}", file=sys.stderr)
    raise SystemExit(1)


def require_keys(value, required, allowed, context):
    if not isinstance(value, dict):
        fail(f"{context} must be an object")
    missing = required - value.keys()
    unknown = value.keys() - allowed
    if missing:
        fail(f"{context} is missing: {', '.join(sorted(missing))}")
    if unknown:
        fail(f"{context} has unknown keys: {', '.join(sorted(unknown))}")


def safe_text(value, context):
    if not isinstance(value, str) or not value.strip():
        fail(f"{context} must be a non-empty string")
    if any(char in value for char in "\t\r\n"):
        fail(f"{context} must not contain tabs or newlines")


def main():
    try:
        config = json.loads(CONFIG_PATH.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        fail(f"cannot read {CONFIG_PATH}: {error}")

    require_keys(
        config,
        {"schema_version", "architecture", "samples"},
        {"schema_version", "architecture", "samples"},
        "root",
    )
    if config["schema_version"] != 1:
        fail("schema_version must be 1")
    if config["architecture"] != "ppu_10":
        fail("architecture must be ppu_10 for the OAM-810E pool")
    if not isinstance(config["samples"], list) or not config["samples"]:
        fail("samples must be a non-empty array")

    binary_dirs = set()
    targets = set()
    asset_names = set()
    case_count = 0

    for sample_index, sample in enumerate(config["samples"]):
        context = f"samples[{sample_index}]"
        require_keys(
            sample,
            {"source", "binary_dir", "cases"},
            {"source", "binary_dir", "cases"},
            context,
        )

        source = sample["source"]
        safe_text(source, f"{context}.source")
        source_path = PurePosixPath(source)
        if source_path.is_absolute() or ".." in source_path.parts:
            fail(f"{context}.source must stay inside the repository")
        cmake_file = REPO_ROOT / source_path / "CMakeLists.txt"
        if not cmake_file.is_file():
            fail(f"{context}.source has no CMakeLists.txt: {source}")
        cmake_text = cmake_file.read_text(encoding="utf-8")

        binary_dir = sample["binary_dir"]
        if not isinstance(binary_dir, str) or not SAFE_NAME.fullmatch(binary_dir):
            fail(f"{context}.binary_dir is not a safe directory name")
        if binary_dir in binary_dirs:
            fail(f"duplicate binary_dir: {binary_dir}")
        binary_dirs.add(binary_dir)

        cases = sample["cases"]
        if not isinstance(cases, list) or not cases:
            fail(f"{context}.cases must be a non-empty array")

        for case_index, case in enumerate(cases):
            case_context = f"{context}.cases[{case_index}]"
            require_keys(
                case,
                {"target", "success", "description", "assets"},
                {"target", "success", "description", "assets"},
                case_context,
            )

            target = case["target"]
            if not isinstance(target, str) or not SAFE_NAME.fullmatch(target):
                fail(f"{case_context}.target is not a safe CMake target name")
            if target in targets:
                fail(f"duplicate target: {target}")
            targets.add(target)
            pattern = re.compile(TARGET_PATTERN.format(re.escape(target)))
            if not pattern.search(cmake_text):
                fail(f"target {target} is not declared by {source}/CMakeLists.txt")

            safe_text(case["success"], f"{case_context}.success")
            safe_text(case["description"], f"{case_context}.description")

            assets = case["assets"]
            if not isinstance(assets, list):
                fail(f"{case_context}.assets must be an array")
            for asset in assets:
                if not isinstance(asset, str) or not SAFE_NAME.fullmatch(asset):
                    fail(f"{case_context}.assets contains an unsafe file name")
                if asset in asset_names:
                    fail(f"duplicate runtime asset name: {asset}")
                asset_names.add(asset)

            case_count += 1

    print(
        f"validated {case_count} CI cases from {len(config['samples'])} sample directories "
        f"for {config['architecture']}"
    )


if __name__ == "__main__":
    main()
