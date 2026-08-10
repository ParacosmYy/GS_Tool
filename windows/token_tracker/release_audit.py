"""Read-only release readiness audit for the local checkout.

Author: AI Token Tracker Engineering Team
Maintainer: Project Owner
Purpose: Collect reproducible source and environment gate evidence without
         creating databases, users, build artifacts, or deployment state.
Module: Delivery / release audit application boundary
"""

from __future__ import annotations

import ast
from dataclasses import asdict, dataclass
import hashlib
from html.parser import HTMLParser
import json
from importlib.util import find_spec
import os
from pathlib import Path
import re
import shutil
from typing import Any, Iterable


PASS = "pass"
PENDING = "pending"
FAIL = "fail"
SOURCE_EXTENSIONS = frozenset({
    ".bat", ".css", ".gradle", ".html", ".java", ".js", ".json", ".kt", ".kts", ".py", ".ps1", ".xml",
})
LINE_CAP_EXTENSIONS = SOURCE_EXTENSIONS | frozenset({".md", ".properties", ".toml", ".txt", ".yaml", ".yml"})
SOURCE_IGNORED_DIRS = frozenset({
    ".cache",
    ".git",
    ".venv",
    "__pycache__",
    "backups",
    "build",
    "data",
    "dist",
    "staging",
    ".toolchain",
})
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
    _check_web_ui_contract(project_root, checks)
    _check_source_line_cap(project_root, checks)
    _check_source_headers(project_root, checks)
    _check_public_api_docstrings(project_root, checks)
    _check_android_kdoc(project_root, checks)
    _check_architecture_boundaries(project_root, checks)
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
        "windows/docs/release-readiness.md",
        "windows/skills/README.md",
        "windows/skills/project-ui-orchestration/SKILL.md",
        "windows/skills/ci-cd-and-automation/SKILL.md",
        "windows/roles/README.md",
        "windows/ui-modules/01-shell/README.md",
        "windows/ui-modules/02-auth/README.md",
        "windows/ui-modules/03-observatory/README.md",
        "windows/ui-modules/04-connect/README.md",
        "windows/ui-modules/05-history/README.md",
        "windows/roles/01-ui-director/README.md",
        "windows/roles/02-dev-shell-auth/README.md",
        "windows/roles/03-dev-observatory/README.md",
        "windows/roles/04-dev-connect/README.md",
        "windows/roles/05-dev-runtime/README.md",
        "windows/roles/06-architect-system/README.md",
        "windows/roles/07-architect-delivery/README.md",
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
    files = _line_cap_files(root)
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
        checks.append(AuditCheck("source-line-cap", PASS, f"已扫描 {len(files)} 个代码/文本文件"))


def _source_files(root: Path) -> list[Path]:
    """Return authored source files while excluding generated checkout state."""

    return _files_with_extensions(root, SOURCE_EXTENSIONS)


def _line_cap_files(root: Path) -> list[Path]:
    """Return authored and documented text files covered by the 1000-line cap."""

    return _files_with_extensions(root, LINE_CAP_EXTENSIONS)


def _files_with_extensions(root: Path, extensions: frozenset[str]) -> list[Path]:
    """Return readable candidate files while excluding generated checkout state."""

    return [
        path
        for path in root.rglob("*")
        if path.is_file()
        and path.suffix.casefold() in extensions
        and not SOURCE_IGNORED_DIRS.intersection(
            part.casefold() for part in path.relative_to(root).parts
        )
    ]


def _check_source_headers(root: Path, checks: list[AuditCheck]) -> None:
    """Require the project author header on owned source files, not vendors."""

    missing: list[str] = []
    for path in _source_files(root):
        relative_parts = {part.casefold() for part in path.relative_to(root).parts}
        if "vendor" in relative_parts:
            continue
        try:
            head = "\n".join(path.read_text(encoding="utf-8").splitlines()[:12])
        except (OSError, UnicodeDecodeError):
            missing.append("unreadable")
            continue
        if "Author:" not in head and "作者：" not in head and "作者:" not in head:
            missing.append(path.as_posix())
    if missing:
        checks.append(AuditCheck("source-headers", FAIL, f"{len(missing)} 个文件缺少作者头"))
    else:
        checks.append(AuditCheck("source-headers", PASS, "自有源文件均包含作者头（vendor 已排除）"))


def _check_public_api_docstrings(root: Path, checks: list[AuditCheck]) -> None:
    """Require docstrings on public module definitions and class methods."""

    missing: list[str] = []
    package_root = root / "windows/token_tracker"
    for path in sorted(package_root.glob("*.py")):
        try:
            tree = ast.parse(path.read_text(encoding="utf-8"), filename=str(path))
        except (OSError, UnicodeDecodeError, SyntaxError):
            missing.append(path.relative_to(root).as_posix())
            continue
        for node in tree.body:
            if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)):
                if not node.name.startswith("_") and ast.get_docstring(node) is None:
                    missing.append(f"{path.name}:{node.lineno}:{node.name}")
                if isinstance(node, ast.ClassDef):
                    for member in node.body:
                        if (
                            isinstance(member, (ast.FunctionDef, ast.AsyncFunctionDef))
                            and not member.name.startswith("_")
                            and ast.get_docstring(member) is None
                        ):
                            missing.append(f"{path.name}:{member.lineno}:{node.name}.{member.name}")
    if missing:
        checks.append(AuditCheck("public-api-docstrings", FAIL, f"{len(missing)} 个公开接口缺少 docstring"))
    else:
        checks.append(AuditCheck("public-api-docstrings", PASS, "模块公开接口与公开类方法均有 docstring"))


def _check_android_kdoc(root: Path, checks: list[AuditCheck]) -> None:
    """Require KDoc on public Android classes and functions in the app source."""

    declaration = re.compile(
        r"^(?P<indent>\s*)(?P<modifiers>(?:(?:public|private|internal|protected|override|"
        r"suspend|data|sealed|enum|abstract|open|inline|operator|infix|tailrec)\s+)*)"
        r"(?P<kind>class|object|interface|fun)\s+(?P<name>[A-Za-z_]\w*)"
    )
    source_root = root / "android/app/src/main/java"
    missing: list[str] = []
    for path in sorted(source_root.rglob("*.kt")):
        try:
            lines = path.read_text(encoding="utf-8").splitlines()
        except (OSError, UnicodeDecodeError):
            missing.append(path.relative_to(root).as_posix())
            continue
        for index, line in enumerate(lines):
            match = declaration.match(line)
            if not match:
                continue
            modifiers = match.group("modifiers") or ""
            if any(f"{visibility} " in modifiers for visibility in ("private", "internal", "protected")):
                continue
            if modifiers.strip() == "override":
                continue
            if not _has_kdoc(lines, index):
                relative = path.relative_to(root).as_posix()
                missing.append(f"{relative}:{index + 1}:{match.group('name')}")
    if missing:
        checks.append(AuditCheck("android-kdoc", FAIL, f"{len(missing)} 个 Android 公开声明缺少 KDoc"))
    else:
        checks.append(AuditCheck("android-kdoc", PASS, "Android 公开类与函数均有 KDoc"))


def _has_kdoc(lines: list[str], declaration_index: int) -> bool:
    """Check for a KDoc block immediately before a declaration and annotations."""

    cursor = declaration_index - 1
    while cursor >= 0:
        stripped = lines[cursor].strip()
        if not stripped or stripped.startswith("@") or stripped.startswith("override "):
            cursor -= 1
            continue
        return stripped.endswith("*/")
    return False


def _check_architecture_boundaries(root: Path, checks: list[AuditCheck]) -> None:
    """Verify that UI surfaces do not bypass the Android/data and Python layers."""

    violations: list[str] = []
    android_root = root / "android/app/src/main/java"
    forbidden_android_imports = re.compile(
        r"import .*?(?:java\.net|android\.database|androidx\.room|data\.remote|data\.secure|HttpURLConnection)"
    )
    for path in sorted(android_root.rglob("*.kt")):
        normalized = path.as_posix()
        is_ui_surface = (
            "/feature/admin/" in normalized
            or "/feature/dashboard/" in normalized
            or path.name == "LoginScreen.kt"
            or "/ui/" in normalized
            or path.name == "MainActivity.kt"
        )
        if not is_ui_surface:
            continue
        for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
            if forbidden_android_imports.search(line):
                violations.append(f"{path.name}:{line_number}")

    python_rules = {
        "services.py": ("flask", "sqlite3"),
        "providers.py": ("flask", "sqlite3"),
        "db.py": ("flask",),
        "web.py": ("requests",),
    }
    python_root = root / "windows/token_tracker"
    for filename, forbidden_modules in python_rules.items():
        path = python_root / filename
        for line_number, line in enumerate(path.read_text(encoding="utf-8").splitlines(), start=1):
            if any(
                re.search(rf"(?:from|import)\s+{re.escape(module)}(?:\.|\s|$)", line)
                for module in forbidden_modules
            ):
                violations.append(f"{filename}:{line_number}")
    if violations:
        checks.append(AuditCheck("architecture-boundaries", FAIL, f"{len(violations)} 个层间依赖越界"))
    else:
        checks.append(AuditCheck("architecture-boundaries", PASS, "Android UI/Python 应用层未绕过数据与网络边界"))


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
        ("project-skills-source", "windows/skills/README.md", "7676817c12a1317454ae3898a0c5c1eacf5dd3d5"),
        ("project-ui-orchestration", "windows/skills/project-ui-orchestration/SKILL.md", "AI Token Tracker UI"),
        ("project-ci-cd", "windows/skills/ci-cd-and-automation/SKILL.md", "Project CI/CD and Automation Skill"),
        ("role-framework", "windows/roles/README.md", "七角色独立交付区"),
        ("ui-module-framework", "windows/ui-modules/01-shell/README.md", "UI Module 01"),
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


class _UiMarkupContractParser(HTMLParser):
    """Collect the markup facts required by the static UI accessibility gate."""

    def __init__(self) -> None:
        super().__init__(convert_charrefs=True)
        self.label_depth = 0
        self.images_without_alt: list[str] = []
        self.controls_without_label: list[str] = []
        self.tables_without_caption = 0
        self.tables_without_scoped_heading = 0
        self._tables: list[dict[str, bool]] = []

    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        """Record accessibility-relevant start tags without executing templates."""

        normalized = tag.casefold()
        attributes = {name.casefold(): value for name, value in attrs}
        if normalized == "label":
            self.label_depth += 1
        elif normalized == "img" and "alt" not in attributes:
            self.images_without_alt.append("img")
        elif normalized in {"input", "textarea", "select"}:
            input_type = str(attributes.get("type", "")).casefold()
            if input_type != "hidden" and self.label_depth == 0:
                self.controls_without_label.append(normalized)
        elif normalized == "table":
            self._tables.append({"caption": False, "scoped_heading": False})
        elif normalized == "caption" and self._tables:
            self._tables[-1]["caption"] = True
        elif normalized == "th" and self._tables:
            scope = str(attributes.get("scope", "")).casefold()
            if scope in {"col", "row", "colgroup", "rowgroup"}:
                self._tables[-1]["scoped_heading"] = True

    def handle_endtag(self, tag: str) -> None:
        """Close tracked labels and tables as the parser reaches their end tags."""

        normalized = tag.casefold()
        if normalized == "label":
            self.label_depth = max(0, self.label_depth - 1)
        elif normalized == "table" and self._tables:
            table = self._tables.pop()
            if not table["caption"]:
                self.tables_without_caption += 1
            if not table["scoped_heading"]:
                self.tables_without_scoped_heading += 1


def _check_web_ui_contract(root: Path, checks: list[AuditCheck]) -> None:
    """Verify static markup and motion fallbacks before a Web release."""

    template_root = root / "windows/token_tracker/templates"
    templates = sorted(template_root.glob("*.html"))
    problems: list[str] = []
    parsed_templates = 0
    for path in templates:
        try:
            source = path.read_text(encoding="utf-8")
            parser = _UiMarkupContractParser()
            parser.feed(source)
            parser.close()
        except (OSError, UnicodeDecodeError) as exc:
            problems.append(f"{path.name}: unreadable ({type(exc).__name__})")
            continue
        parsed_templates += 1
        problems.extend(f"{path.name}: image missing alt" for _ in parser.images_without_alt)
        problems.extend(f"{path.name}: {control} outside label" for control in parser.controls_without_label)
        if parser.tables_without_caption:
            problems.append(f"{path.name}: table missing caption")
        if parser.tables_without_scoped_heading:
            problems.append(f"{path.name}: table missing scoped heading")

    required_fragments = (
        ("base.html", 'href="#main-content"'),
        ("base.html", 'class="story-backdrop-image"'),
        ("base.html", 'aria-hidden="true"'),
        ("login.html", "<h1"),
        ("register.html", "<h1"),
    )
    for filename, fragment in required_fragments:
        path = template_root / filename
        try:
            if fragment not in path.read_text(encoding="utf-8"):
                problems.append(f"{filename}: missing {fragment}")
        except (OSError, UnicodeDecodeError):
            problems.append(f"{filename}: required fragment unreadable")

    css_sources = []
    for relative in (
        "windows/token_tracker/static/style.css",
        "windows/token_tracker/static/ui-polish.css",
        "windows/token_tracker/static/scene-motion.css",
    ):
        try:
            css_sources.append((root / relative).read_text(encoding="utf-8"))
        except (OSError, UnicodeDecodeError):
            problems.append(f"{relative}: stylesheet unreadable")
    css = "\n".join(css_sources)
    for fragment in (":focus-visible", "prefers-reduced-motion", "forced-colors: active"):
        if fragment not in css:
            problems.append(f"stylesheets: missing {fragment}")

    if problems:
        checks.append(AuditCheck("web-ui-contracts", FAIL, f"{len(problems)} 个 UI 契约问题"))
    else:
        checks.append(AuditCheck("web-ui-contracts", PASS, f"已核对 {parsed_templates} 个模板和无障碍动效降级"))


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
