"""Self-tests for EmbedDebug Python tool entrypoints."""

from __future__ import annotations

import contextlib
import io
import importlib.util
import os
import sys
import tempfile
import unittest
from pathlib import Path
from zipfile import ZipFile

from tools import package_embeddebug, start_embeddebug, verify_package_embeddebug


def load_project_audit_module():
    """加载连字符目录下的 project_audit 工具模块。"""
    module_path = start_embeddebug.repo_root() / "tools" / "project-audit" / "project_audit.py"
    spec = importlib.util.spec_from_file_location("project_audit", module_path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load project audit module: {module_path}")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


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


class VerifyPackageEmbedDebugToolTest(unittest.TestCase):
    def test_missing_package_dir_reports_failure(self) -> None:
        ok, messages = verify_package_embeddebug.verify_package_dir(Path("missing-package"))

        self.assertFalse(ok)
        self.assertEqual(messages, ["missing: package directory missing-package"])

    def test_verify_package_dir_reports_required_files(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            package_dir = Path(temp_dir) / "EmbedDebug-dev-windows-x64"
            for relative_path in verify_package_embeddebug.REQUIRED_RELATIVE_PATHS:
                target = package_dir / relative_path
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_text("placeholder", encoding="utf-8")

            ok, messages = verify_package_embeddebug.verify_package_dir(package_dir)

        self.assertTrue(ok)
        self.assertIn("ok: EmbedDebug.exe", messages)
        self.assertIn("ok: platforms/qwindows.dll", messages)

    def test_verify_package_dir_lists_missing_files(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            package_dir = Path(temp_dir) / "EmbedDebug-dev-windows-x64"
            package_dir.mkdir()
            (package_dir / "EmbedDebug.exe").write_text("placeholder", encoding="utf-8")

            ok, messages = verify_package_embeddebug.verify_package_dir(package_dir)

        self.assertFalse(ok)
        self.assertIn("ok: EmbedDebug.exe", messages)
        self.assertIn("missing: platforms/qwindows.dll", messages)

    def test_latest_package_dir_selects_newest_candidate(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            dist_root = root / "dist"
            old_dir = dist_root / "EmbedDebug-old-windows-x64"
            new_dir = dist_root / "EmbedDebug-new-windows-x64"
            ignored_dir = dist_root / "OtherTool-new-windows-x64"
            old_dir.mkdir(parents=True)
            new_dir.mkdir()
            ignored_dir.mkdir()

            old_time = 1_700_000_000
            new_time = old_time + 60
            old_dir.touch()
            new_dir.touch()
            ignored_dir.touch()

            os.utime(old_dir, (old_time, old_time))
            os.utime(new_dir, (new_time, new_time))
            os.utime(ignored_dir, (new_time + 60, new_time + 60))

            latest = verify_package_embeddebug.latest_package_dir(root)

        self.assertEqual(latest.name, "EmbedDebug-new-windows-x64")


class ProjectAuditToolTest(unittest.TestCase):
    def test_cmake_entries_reads_included_cmake_source_list(self) -> None:
        project_audit = load_project_audit_module()

        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            cmake_dir = root / "cmake"
            src_dir = root / "src"
            cmake_dir.mkdir()
            src_dir.mkdir()
            (root / "CMakeLists.txt").write_text(
                "include(cmake/EmbedDebugSources.cmake)\n",
                encoding="utf-8",
            )
            (cmake_dir / "EmbedDebugSources.cmake").write_text(
                "set(SOURCES\n    src/main.cpp\n)\n",
                encoding="utf-8",
            )
            (src_dir / "main.cpp").write_text("int main() { return 0; }\n", encoding="utf-8")

            entries = project_audit.cmake_entries(root)

        self.assertIn("src/main.cpp", entries)


def main() -> int:
    suite = unittest.defaultTestLoader.loadTestsFromModule(sys.modules[__name__])
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    raise SystemExit(main())
