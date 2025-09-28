#!/usr/bin/env python3
"""Try PlatformIO uploads across environments until the target accepts the firmware."""
from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Iterable, List

# Error fragments that indicate the wrong chip/environment combination
CHIP_MISMATCH_MARKERS: tuple[str, ...] = (
    "Wrong --chip argument",
    "This chip is ESP32",
    "This chip is ESP8266",
)


def resolve_platformio(cmd: str | None) -> str:
    """Locate the PlatformIO executable following common install locations."""
    candidates: List[str] = []
    if cmd:
        candidates.append(cmd)
        resolved = shutil.which(cmd)
        if resolved:
            candidates.append(resolved)

    home = Path.home()
    candidates.extend(
        [
            str(home / ".platformio" / "penv" / "bin" / "platformio"),
            str(home / ".local" / "bin" / "platformio"),
        ]
    )

    seen: set[str] = set()
    for candidate in candidates:
        expanded = os.path.expanduser(candidate)
        if expanded in seen:
            continue
        seen.add(expanded)
        if os.path.isfile(expanded) and os.access(expanded, os.X_OK):
            return expanded

    raise FileNotFoundError(
        "Could not locate the PlatformIO executable. Install PlatformIO or pass --platformio with an explicit path."
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "-e",
        "--env",
        action="append",
        dest="envs",
        required=True,
        help="Environment name to try (ordered, first match wins)",
    )
    parser.add_argument(
        "--port",
        dest="port",
        default=None,
        help="Serial port to forward to PlatformIO (optional)",
    )
    parser.add_argument(
        "--platformio",
        dest="pio",
        default=os.environ.get("PLATFORMIO_CMD", "platformio"),
        help="PlatformIO executable to invoke (default: %(default)s)",
    )
    return parser.parse_args()


def dedupe(seq: Iterable[str]) -> List[str]:
    seen: set[str] = set()
    ordered: List[str] = []
    for item in seq:
        if item and item not in seen:
            seen.add(item)
            ordered.append(item)
    return ordered


def looks_like_chip_mismatch(output: str) -> bool:
    return any(marker in output for marker in CHIP_MISMATCH_MARKERS)


def platformio_upload(pio_cmd: str, env: str, port: str | None) -> subprocess.CompletedProcess[str]:
    cmd = [pio_cmd, "run", "--target", "upload", "--environment", env]
    if port:
        cmd += ["--upload-port", port]

    print(f"-> Trying upload with environment '{env}'", flush=True)
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.stdout:
        sys.stdout.write(result.stdout)
    if result.stderr:
        sys.stderr.write(result.stderr)
    return result


def main() -> int:
    args = parse_args()
    envs = dedupe(args.envs)
    try:
        pio_path = resolve_platformio(args.pio)
    except FileNotFoundError as exc:
        print(str(exc), file=sys.stderr)
        return 1

    last_result: subprocess.CompletedProcess[str] | None = None

    for env in envs:
        last_result = platformio_upload(pio_path, env, args.port)
        if last_result.returncode == 0:
            return 0

        combined = "\n".join(filter(None, [last_result.stdout, last_result.stderr]))
        if not combined or not looks_like_chip_mismatch(combined):
            break

        print(
            f"Environment '{env}' failed due to chip mismatch, trying next option...",
            flush=True,
        )

    if last_result is None:
        print("No environments provided.", file=sys.stderr)
        return 1

    return last_result.returncode if last_result.returncode is not None else 1


if __name__ == "__main__":
    sys.exit(main())
