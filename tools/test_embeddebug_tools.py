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

REPO_ROOT = Path(__file__).resolve().parents[1]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from embeddebug.devtools import package_pyinstaller, verify_pyinstaller_package
from tools import start_embeddebug


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
        # Batch 101: 接受不同克隆目录名（GS_Tool 或 User_Serial）。
        self.assertIn(root.name, ("GS_Tool", "User_Serial"))

    def test_dry_run_prints_python_uv_command_without_launching(self) -> None:
        output = io.StringIO()

        with contextlib.redirect_stdout(output):
            exit_code = start_embeddebug.main(["--dry-run"])

        self.assertEqual(exit_code, 0)
        self.assertIn("uv run start-embeddebug", output.getvalue())
        self.assertNotIn("EmbedDebug.bat", output.getvalue())


class PackageEmbedDebugToolTest(unittest.TestCase):
    def test_package_name_sanitizes_version_text(self) -> None:
        name = package_pyinstaller.package_name("dev build/001")

        self.assertEqual(name, "EmbedDebugPy-dev-build-001-windows-x64")

    def test_pyinstaller_command_has_no_cpp_toolchain_steps(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            entry_script = root / "python" / "embeddebug" / "app" / "main.py"
            entry_script.parent.mkdir(parents=True)
            entry_script.write_text("raise SystemExit(0)\n", encoding="utf-8")

            command = package_pyinstaller.pyinstaller_command(
                package_pyinstaller.PackageConfig(
                    root=root,
                    version="dev",
                    temp_root=root / ".tmp",
                )
            )

        command_text = " ".join(command).lower()
        self.assertIn("pyinstaller", command_text)
        self.assertNotIn("cmake", command_text)
        self.assertNotIn("windeployqt", command_text)
        self.assertNotIn("build/embeddebug.exe", command_text.replace("\\", "/"))


class VerifyPackageEmbedDebugToolTest(unittest.TestCase):
    def test_missing_package_dir_reports_failure(self) -> None:
        ok, messages = verify_pyinstaller_package.verify_package_dir(Path("missing-package"))

        self.assertFalse(ok)
        self.assertEqual(messages, ["missing: package directory missing-package"])

    def test_verify_package_dir_reports_required_files(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            package_dir = Path(temp_dir) / "EmbedDebug-dev-windows-x64"
            for relative_path in verify_pyinstaller_package.REQUIRED_RELATIVE_PATHS:
                target = package_dir / relative_path
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_text("placeholder", encoding="utf-8")

            ok, messages = verify_pyinstaller_package.verify_package_dir(package_dir)

        self.assertTrue(ok)
        self.assertIn("ok: EmbedDebugPy.exe", messages)
        self.assertIn("ok: _internal/PyQt6/Qt6/plugins/platforms/qwindows.dll", messages)

    def test_verify_package_dir_lists_missing_files(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            package_dir = Path(temp_dir) / "EmbedDebugPy-dev-windows-x64"
            package_dir.mkdir()
            (package_dir / "EmbedDebugPy.exe").write_text("placeholder", encoding="utf-8")

            ok, messages = verify_pyinstaller_package.verify_package_dir(package_dir)

        self.assertFalse(ok)
        self.assertIn("ok: EmbedDebugPy.exe", messages)
        self.assertIn("missing: THIRD_PARTY_NOTICES.md", messages)

    def test_latest_package_dir_selects_newest_candidate(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            dist_root = root / "dist"
            old_dir = dist_root / "EmbedDebugPy-old-windows-x64"
            new_dir = dist_root / "EmbedDebugPy-new-windows-x64"
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

            latest = verify_pyinstaller_package.latest_package_dir(root)

        self.assertEqual(latest.name, "EmbedDebugPy-new-windows-x64")


class ProjectAuditToolTest(unittest.TestCase):
    def test_python_counts_reads_runtime_and_tests(self) -> None:
        project_audit = load_project_audit_module()

        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            runtime = root / "python" / "embeddebug" / "app" / "main.py"
            test_file = root / "tests" / "python" / "unit" / "test_main.py"
            runtime.parent.mkdir(parents=True)
            test_file.parent.mkdir(parents=True)
            runtime.write_text("raise SystemExit(0)\n", encoding="utf-8")
            test_file.write_text("def test_ok(): assert True\n", encoding="utf-8")

            counts = project_audit.python_counts(root)

        self.assertEqual(counts["runtime"], 1)
        self.assertEqual(counts["tests"], 1)


def main() -> int:
    suite = unittest.defaultTestLoader.loadTestsFromModule(sys.modules[__name__])
    result = unittest.TextTestRunner(verbosity=2).run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    raise SystemExit(main())
