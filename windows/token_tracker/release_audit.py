"""Read-only release readiness audit for the local checkout.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Collect reproducible source and environment gate evidence without
         creating databases, users, build artifacts, or deployment state.
Module: Delivery / release audit application boundary
"""

from __future__ import annotations

from dataclasses import asdict, dataclass
import hashlib
import json
from importlib.util import find_spec
import os
from pathlib import Path
import shutil
from typing import Any, Iterable


PASS = "pass"
PENDING = "pending"
FAIL = "fail"
SOURCE_EXTENSIONS = frozenset({".bat", ".css", ".html", ".js", ".json", ".kt", ".py", ".ps1", ".xml"})
EXPECTED_CHARTJS_SHA256 = "206B6E8BB00FC7BBA2C7EE80CA41DB3E9E05BA7BE0AA35ABEBA9CFD5357F5D0E"


@dataclass(frozen=True)
class AuditCheck:
    """One named gate with a stable status and human-readable evidence."""

    name: str
    status: str
    detail: str


def repository_root() -> Path:
    """Resolve the checkout root from the installed package location."""

    return Path(__file__).resolve().parents[2]


def run_audit(root: Path | None = None) -> list[AuditCheck]:
    """Collect read-only release checks for the supplied repository root."""

    project_root = (root or repository_root()).resolve()
    checks: list[AuditCheck] = []
    _check_required_files(project_root, checks)
    _check_contract_references(project_root, checks)
    _check_source_line_cap(project_root, checks)
    _check_scene_assets(project_root, checks)
    _check_chartjs_asset(project_root, checks)
    _check_runtime_dependencies(checks)
    _check_external_tool_gates(project_root, checks)
    return checks


def summarize(checks: Iterable[AuditCheck]) -> dict[str, int]:
    """Return stable counts for CLI, JSON consumers, and release notes."""

    counts = {PASS: 0, PENDING: 0, FAIL: 0}
    for check in checks:
        counts[check.status] = counts.get(check.status, 0) + 1
    return counts


def format_report(checks: list[AuditCheck], as_json: bool = False) -> str:
    """Render an audit without printing paths that could contain secrets."""

    counts = summarize(checks)
    if as_json:
        return json.dumps(
            {"checks": [asdict(check) for check in checks], "summary": counts},
            # ASCII escapes keep redirected output valid regardless of the
            # legacy Windows console code page; CI can decode it as UTF-8.
            ensure_ascii=True,
            indent=2,
        )
    lines = ["AI Token Tracker release audit (read-only)"]
    for check in checks:
        lines.append(f"[{check.status.upper():7}] {check.name}: {check.detail}")
    lines.append(
        f"summary: pass={counts[PASS]} pending={counts[PENDING]} fail={counts[FAIL]}"
    )
    return "\n".join(lines)


def exit_code(checks: Iterable[AuditCheck], strict: bool) -> int:
    """Map audit state to a scriptable exit code without hiding pending gates."""

    counts = summarize(checks)
    if counts[FAIL]:
        return 2
    if strict and counts[PENDING]:
        return 3
    return 0


def _check_required_files(root: Path, checks: list[AuditCheck]) -> None:
    required = (
        "README.md",
        "start.bat",
        "windows/.env.example",
        "windows/requirements.lock",
        "windows/packaging/requirements-build.lock",
        "windows/packaging/toolchain-doctor.ps1",
        "windows/deployment/Caddyfile.example",
        "windows/token_tracker/web.py",
        "windows/token_tracker/api_v1.py",
        "windows/token_tracker/gateway.py",
        "windows/token_tracker/gateway_contracts.py",
        "windows/token_tracker/gateway_queue.py",
        "windows/token_tracker/static/vendor/chart.umd.min.js",
        "windows/token_tracker/static/vendor/CHARTJS-LICENSE.txt",
        "android/app/src/main/AndroidManifest.xml",
        "android/toolchain-doctor.ps1",
    )
    missing = [item for item in required if not (root / item).is_file()]
    if missing:
        checks.append(AuditCheck("required-artifacts", FAIL, f"缺少 {len(missing)} 个交付文件"))
    else:
        checks.append(AuditCheck("required-artifacts", PASS, f"已检查 {len(required)} 个关键文件"))


def _check_source_line_cap(root: Path, checks: list[AuditCheck]) -> None:
    source_roots = (root / "windows/token_tracker", root / "windows/packaging", root / "android/app/src/main")
    files = [
        path
        for source_root in source_roots
        if source_root.is_dir()
        for path in source_root.rglob("*")
        if path.is_file() and path.suffix.casefold() in SOURCE_EXTENSIONS
    ]
    oversized: list[str] = []
    for path in files:
        try:
            lines = sum(1 for _ in path.open("r", encoding="utf-8"))
        except (OSError, UnicodeDecodeError):
            oversized.append("unreadable")
            continue
        if lines > 1000:
            oversized.append(path.name)
    if oversized:
        checks.append(AuditCheck("source-line-cap", FAIL, f"{len(oversized)} 个文件超过 1000 行"))
    else:
        checks.append(AuditCheck("source-line-cap", PASS, f"已扫描 {len(files)} 个源文件"))


def _check_contract_references(root: Path, checks: list[AuditCheck]) -> None:
    """Verify that key decisions are wired into the current source tree."""

    references = (
        ("web-scene-reference", "windows/token_tracker/static/scene-motion.css", "embedded-rust-engineer-bg-v7.png"),
        ("web-scene-image-layer", "windows/token_tracker/templates/base.html", "story-backdrop-image"),
        ("android-scene-reference", "android/app/src/main/java/com/aitokentracker/ui/TokenTrackerApp.kt", "embedded_rust_engineer_bg_v7"),
        ("kimi-provider-presets", "windows/token_tracker/templates/dashboard.html", "kimi-code"),
        ("provider-response-projection", "windows/token_tracker/provider_service.py", "project_response"),
        ("usage-ingest-endpoint", "windows/token_tracker/api_v1.py", "/ingest/usage"),
        ("ingest-token-schema", "windows/token_tracker/schema.py", "usage_ingest_tokens"),
        ("local-gateway-route", "windows/token_tracker/gateway.py", "/v1/chat/completions"),
        ("local-gateway-cli", "windows/token_tracker/cli.py", "cmd_gateway"),
        ("local-gateway-reporter", "windows/token_tracker/gateway_reporting.py", "UsageReporter"),
        ("local-gateway-contract", "windows/token_tracker/gateway_contracts.py", "class UsageReport"),
        ("local-gateway-queue", "windows/token_tracker/gateway_queue.py", "EncryptedUsageQueue"),
        ("chartjs-local-reference", "windows/token_tracker/templates/dashboard.html", "vendor/chart.umd.min.js"),
        ("chartjs-local-csp", "windows/token_tracker/web.py", "script-src 'self';"),
        ("caddy-log-retention", "windows/deployment/Caddyfile.example", "roll_keep 14"),
        ("root-launcher", "start.bat", "call start.bat"),
        ("android-toolchain-doctor", "android/toolchain-doctor.ps1", "Android toolchain pending"),
        ("exe-toolchain-doctor", "windows/packaging/toolchain-doctor.ps1", "EXE packaging pending"),
    )
    missing: list[str] = []
    for name, relative_path, fragment in references:
        path = root / relative_path
        try:
            content = path.read_text(encoding="utf-8")
        except (OSError, UnicodeDecodeError):
            missing.append(name)
            continue
        if fragment not in content:
            missing.append(name)
    if missing:
        checks.append(AuditCheck("contract-references", FAIL, f"{len(missing)} 个关键引用未接入"))
    else:
        checks.append(AuditCheck("contract-references", PASS, f"已核对 {len(references)} 个关键引用"))


def _check_scene_assets(root: Path, checks: list[AuditCheck]) -> None:
    web_asset = root / "windows/token_tracker/static/assets/embedded-rust-engineer-bg-v7.png"
    android_asset = root / "android/app/src/main/res/drawable-nodpi/embedded_rust_engineer_bg_v7.png"
    if not web_asset.is_file() or not android_asset.is_file():
        checks.append(AuditCheck("cross-platform-scene", FAIL, "v7 Web/Android 资产不完整"))
        return
    web_hash = _sha256(web_asset)
    android_hash = _sha256(android_asset)
    if web_hash != android_hash:
        checks.append(AuditCheck("cross-platform-scene", FAIL, "v7 Web/Android SHA-256 不一致"))
        return
    checks.append(AuditCheck("cross-platform-scene", PASS, f"v7 SHA-256 一致 {web_hash[:12]}…"))


def _check_chartjs_asset(root: Path, checks: list[AuditCheck]) -> None:
    """Pin the vendored chart runtime to the reviewed release artifact."""

    asset = root / "windows/token_tracker/static/vendor/chart.umd.min.js"
    if not asset.is_file():
        checks.append(AuditCheck("chartjs-supply-chain", FAIL, "本地 Chart.js 资源缺失"))
        return
    actual = _sha256(asset)
    if actual != EXPECTED_CHARTJS_SHA256:
        checks.append(AuditCheck("chartjs-supply-chain", FAIL, "Chart.js SHA-256 与 ADR-051 不一致"))
        return
    checks.append(AuditCheck("chartjs-supply-chain", PASS, f"Chart.js 4.4.7 SHA-256 {actual[:12]}…"))


def _check_runtime_dependencies(checks: list[AuditCheck]) -> None:
    required = ("flask", "requests", "dotenv", "waitress")
    missing = [name for name in required if find_spec(name) is None]
    if missing:
        checks.append(AuditCheck("python-runtime", FAIL, f"缺少 {', '.join(missing)}"))
    else:
        checks.append(AuditCheck("python-runtime", PASS, "Flask/requests/dotenv/Waitress 可导入"))


def _check_external_tool_gates(root: Path, checks: list[AuditCheck]) -> None:
    pyinstaller_ready = find_spec("PyInstaller") is not None
    checks.append(
        AuditCheck(
            "exe-toolchain",
            PASS if pyinstaller_ready else PENDING,
            "PyInstaller 可用" if pyinstaller_ready else "PyInstaller 未安装，等待批准构建环境",
        )
    )
    gradle_ready = (root / "android/gradlew.bat").is_file() or shutil.which("gradle") is not None
    java_ready = _java_runtime_available()
    sdk_root = _android_sdk_root(root)
    sdk_ready = bool(
        sdk_root
        and (sdk_root / "platforms/android-37/android.jar").is_file()
        and any((sdk_root / "build-tools").glob("*/aapt2.exe"))
    )
    android_missing = [
        label
        for label, available in (
            ("JDK", java_ready),
            ("Gradle wrapper/命令", gradle_ready),
            ("Android SDK API 37/build-tools", sdk_ready),
        )
        if not available
    ]
    checks.append(
        AuditCheck(
            "android-toolchain",
            PASS if not android_missing else PENDING,
            "JDK、Gradle、Android SDK API 37 与 build-tools 可用"
            if not android_missing
            else f"等待 {', '.join(android_missing)}",
        )
    )
    caddy_ready = shutil.which("caddy") is not None
    checks.append(
        AuditCheck(
            "edge-toolchain",
            PASS if caddy_ready else PENDING,
            "Caddy 可用" if caddy_ready else "Caddy 未安装，正式 edge validate 待部署主机",
        )
    )


def _java_runtime_available() -> bool:
    """Detect a usable Java launcher without mutating PATH or installing tools."""

    if shutil.which("java"):
        return True
    java_home = os.getenv("JAVA_HOME", "").strip()
    if not java_home:
        return False
    launcher = Path(java_home) / "bin" / ("java.exe" if os.name == "nt" else "java")
    return launcher.is_file()


def _android_sdk_root(root: Path) -> Path | None:
    """Resolve an existing SDK location in env or the project-local reserve."""

    candidates = (
        os.getenv("ANDROID_SDK_ROOT", "").strip(),
        os.getenv("ANDROID_HOME", "").strip(),
        str(root / "android/.toolchain/android-sdk"),
    )
    for candidate in candidates:
        if candidate:
            path = Path(candidate).expanduser()
            if path.is_dir():
                return path
    return None


def _sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest().upper()
