"""Self-tests for EmbedDebug Python tool entrypoints."""

from __future__ import annotations

import contextlib
import io
import sys
import tempfile
import unittest
from pathlib import Path
from zipfile import ZipFile

from tools import package_embeddebug, start_embeddebug


class StartEmbedDebugToolTest(unittest.TestCase):
    def test_repo_root_contains_bat(self) -> None:
        root = start_embeddebug.repo_root()

        self.assertTrue((root / "EmbedDebug.bat").is_file())
        self.assertEqual(root.name, "GS_Tool")

    def test_embeddebug_bat_resolves_root_entrypoint(self) -> None:
        bat_path = start_embeddebug.embeddebug_bat(start_embeddebug.repo_root())

        self.assertEqual(bat_path.name, "EmbedDebug.bat")
        self.assertTrue(bat_path.is_file())

    def test_dry_run_prints_bat_command_without_launching(self) -> None:
        output = io.StringIO()

        with contextlib.redirect_stdout(output):
            exit_code = start_embeddebug.main(["--dry-run"])

        self.assertEqual(exit_code, 0)
        self.assertIn("cmd.exe /c", output.getvalue())
        self.assertIn("EmbedDebug.bat", output.getvalue())


class PackageEmbedDebugToolTest(unittest.TestCase):
    def test_package_name_sanitizes_version_text(self) -> None:
        name = package_embeddebug.package_name("dev build/001")

        self.assertEqual(name, "EmbedDebug-dev-build-001-windows-x64")

    def test_read_local_env_parses_bat_assignments(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            env_file = root / "local_env.bat"
            env_file.write_text(
                '\n'.join(
                    [
                        "@echo off",
                        'set "QT_PREFIX=C:\\Qt"',
                        'set "MINGW_BIN=C:\\mingw64\\bin"',
                    ]
                ),
                encoding="utf-8",
            )

            env_map = package_embeddebug.read_local_env(root)

        self.assertEqual(env_map["QT_PREFIX"], "C:\\Qt")
        self.assertEqual(env_map["MINGW_BIN"], "C:\\mingw64\\bin")

    def test_create_zip_archives_package_contents(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            package_dir = Path(temp_dir) / "EmbedDebug-dev-windows-x64"
            nested_dir = package_dir / "docs"
            nested_dir.mkdir(parents=True)
            (package_dir / "EmbedDebug.exe").write_text("fake exe", encoding="utf-8")
            (nested_dir / "README.txt").write_text("docs", encoding="utf-8")

            zip_path = package_embeddebug.create_zip(package_dir)

            self.assertTrue(zip_path.is_file())
            with ZipFile(zip_path) as archive:
                names = set(archive.namelist())

        self.assertIn("EmbedDebug-dev-windows-x64/EmbedDebug.exe", names)
        self.assertIn("EmbedDebug-dev-windows-x64/docs/README.txt", names)


def main() -> int:
    suite = unittest.defaultTestLoader.loadTestsFromModule(sys.modules[__name__])
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    raise SystemExit(main())
