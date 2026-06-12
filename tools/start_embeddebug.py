"""Start EmbedDebug through the repository batch entrypoint."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def embeddebug_bat(root: Path) -> Path:
    path = root / "EmbedDebug.bat"
    if not path.is_file():
        raise RuntimeError(f"EmbedDebug.bat not found: {path}")
    return path


def start_app(args: argparse.Namespace) -> int:
    root = repo_root()
    bat_path = embeddebug_bat(root)
    command = ["cmd.exe", "/c", str(bat_path)]

    if args.dry_run:
        print(" ".join(command))
        return 0

    process = subprocess.Popen(command, cwd=root)
    if args.wait:
        return process.wait()

    print(f"started EmbedDebug.bat pid={process.pid}")
    return 0


def parse_args(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dry-run", action="store_true", help="print the launch command")
    parser.add_argument("--wait", action="store_true", help="wait for the batch process")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    try:
        return start_app(parse_args(argv or sys.argv[1:]))
    except RuntimeError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
