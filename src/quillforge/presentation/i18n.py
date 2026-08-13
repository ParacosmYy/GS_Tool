"""Small presentation-only translation catalog for the desktop shell."""

from __future__ import annotations

import re
from collections.abc import Mapping
from typing import cast

from ..domain.models import Locale

DEFAULT_LOCALE: Locale = "zh-CN"

_ENGLISH: dict[str, str] = {
    "app.title": "QuillForge",
    "document.untitled": "Untitled",
    "menu.file": "&File",
    "menu.edit": "&Edit",
    "menu.tools": "&Tools",
    "menu.help": "&Help",
    "toolbar.command_rail": "✦ Command rail",
    "toolbar.new": "New",
    "toolbar.open": "Open",
    "toolbar.save": "Save",
    "toolbar.find": "Find",
    "toolbar.replace": "Replace",
    "toolbar.command_palette": "Command palette",
    "toolbar.workspace": "Workspace",
    "toolbar.context": "✦ Local workspace · safe by default",
    "workspace.dock": "✦ Explorer",
    "workspace.eyebrow": "Project files",
    "workspace.tree": "Workspace files",
    "workspace.path": "Current workspace path",
    "workspace.empty": "Choose a folder to browse your project.",
    "workspace.empty_directory": "This folder is empty.",
    "workspace.no_selection": "No workspace selected",
    "workspace.choose": "Choose a folder to browse your project.",
    "workspace.open_folder": "Open project folder",
    "workspace.open_file": "Open file",
    "workspace.open_folder_hint": "Select a folder to browse project files.",
    "workspace.open_file_hint": "Choose one file to open directly.",
    "workspace.up": "Up",
    "workspace.up_hint": "Go to the parent folder.",
    "workspace.cancel": "Cancel",
    "workspace.cancel_hint": "Cancel the current workspace operation.",
    "workspace.loading": "Loading workspace…",
    "workspace.entries": "{count} items{suffix} · click files / double-click folders",
    "workspace.truncated_suffix": " · list truncated",
    "workspace.more_entries": "More entries are hidden (limit reached)",
    "workspace.entry_file_hint": "File · click to open",
    "workspace.entry_directory_hint": "Folder · double-click to open",
    "workspace.entry_inaccessible_hint": "Unavailable item",
    "find.find": "Find",
    "find.replace": "Replace",
    "find.case": "Case",
    "find.previous": "Previous",
    "find.next": "Next",
    "find.replace_one": "Replace",
    "find.replace_all": "Replace All",
    "find.cancel": "Cancel",
    "find.close": "Close",
    "find.placeholder": "Find",
    "find.replace_placeholder": "Replace",
    "find.status.operation": "Another editor operation is in progress",
    "find.status.enter": "Enter text to find",
    "find.status.match": "Match found",
    "find.status.none": "No matches",
    "find.status.first": "Find a match first",
    "find.status.replaced_one": "Replaced 1 match",
    "find.status.preparing": "Preparing Replace All...",
    "find.status.counting": "Counting matches...",
    "find.status.cancelled_changed": "Replace All cancelled because the document changed",
    "find.status.counting_progress": "Counting matches... {count}",
    "find.status.replacing_progress": "Replacing matches... {count}",
    "find.status.failed_restored": "Replace All failed; document was restored",
    "find.status.failed_partial": "Replace All failed; document may contain partial changes",
    "find.status.limit": "Stopped: more than {limit} matches; document unchanged",
    "find.error.limit": "Replace All is limited to {limit} matches.",
    "find.status.cancelled_unchanged": "Replace All cancelled; document unchanged",
    "find.status.replaced_count": "Replaced {count} matches",
    "command_palette.title": "Command Palette",
    "command_palette.placeholder": "Search commands by name or ID",
    "command_palette.hint": "Enter to run · Esc to close",
    "command_palette.results": "Command results",
    "command_palette.empty.initial": "No commands are available",
    "command_palette.empty.no_matches": "No commands match this search",
    "search.title": "Find in Files",
    "search.placeholder": "Search text",
    "search.case": "Case sensitive",
    "search.search": "Search",
    "search.cancel": "Cancel",
    "search.close": "Close",
    "search.results": "Workspace search results",
    "search.initial": "Enter text to search the selected workspace",
    "search.empty.initial": "Enter a query to search this workspace",
    "search.empty.loading": "Searching this workspace…",
    "search.empty.no_matches": "No matches in the selected workspace",
    "search.empty.cancelled": "Search cancelled; run another query to refresh results",
    "search.empty.error": "Results are unavailable; correct the query and try again",
    "search.root": "Workspace search root: {root}",
    "search.loading": "Searching workspace…",
    "search.cancelling": "Cancelling workspace search…",
    "search.enter": "Enter text to search",
    "search.failed": "Workspace search failed: {message}",
    "search.cancelled": "Workspace search cancelled; current results kept",
    "search.summary.complete": (
        "Search complete: {matches} matches in {files} files; {bytes} bytes scanned{diagnostics}"
    ),
    "search.summary.cancelled": (
        "Search cancelled: {matches} matches in {files} files; {bytes} bytes scanned{diagnostics}"
    ),
    "search.summary.limited": (
        "Search limited ({reason}): {matches} matches in {files} files; "
        "{bytes} bytes scanned{diagnostics}"
    ),
    "search.summary.diagnostics": " ({count} diagnostics)",
    "search.summary.diagnostics_truncated": " (diagnostics truncated)",
    "search.diagnostics": "Diagnostics",
    "search.diagnostics_count": "Diagnostics ({count})",
    "search.diagnostics_truncated": "Diagnostics ({count} shown; additional records omitted)",
    "search.more_diagnostics": (
        "Additional diagnostics were omitted by the configured ledger bound."
    ),
    "search.outside_workspace": "<outside selected workspace>",
    "settings.title": "QuillForge Settings",
    "settings.appearance_group": "Appearance",
    "settings.editor_group": "Editor",
    "settings.language": "Language",
    "settings.language_english": "English",
    "settings.language_chinese": "简体中文",
    "settings.theme": "Theme",
    "settings.theme_ink": "Ink · Violet",
    "settings.theme_paper": "Paper · Sand",
    "settings.theme_sakura": "Sakura · Pop",
    "settings.accent": "Accent color",
    "settings.accent_violet": "Violet",
    "settings.accent_cyan": "Cyan",
    "settings.accent_rose": "Rose",
    "settings.accent_amber": "Amber",
    "settings.ui_font": "Interface font",
    "settings.ui_font_size": "Interface size",
    "settings.font_size_suffix": " pt",
    "settings.ui_font_style": "Interface style",
    "settings.editor_font": "Editor font",
    "settings.editor_font_size": "Editor font size",
    "settings.editor_font_style": "Editor style",
    "settings.font_style_regular": "Regular",
    "settings.font_style_semibold": "Semibold",
    "settings.font_style_bold": "Bold",
    "settings.font_style_italic": "Italic",
    "settings.wrap": "Wrap long lines",
    "settings.line_numbers": "Show line numbers",
    "settings.motion": "Enable smooth transitions",
    "settings.apply_note": "Language and theme changes apply after saving.",
    "settings.ok": "Save",
    "settings.cancel": "Cancel",
    "settings.restore_defaults": "Restore defaults",
    "settings.restore_defaults_hint": "Reset this draft only; click Save to keep it.",
    "settings.draft.clean": "No unsaved setting changes.",
    "settings.draft.changed": "Unsaved setting changes · Save to apply.",
    "settings.font_status": (
        'Interface font "{ui_font}": {ui_status}; editor font "{editor_font}": '
        + "{editor_status}."
    ),
    "settings.font_status_installed": "installed",
    "settings.font_status_fallback": "not installed; system fallback",
    "settings.font_status_unknown": "availability unavailable",
    "settings.unsupported_font_note": "Unavailable fonts fall back to the system default.",
    "settings.preview.title": "Live preview",
    "settings.preview.accent": "Accent",
    "settings.preview.sample": "✦ QuillForge  ·  Aa  你好",
    "settings.preview.editor_sample": "def forge(): 你好",
    "settings.preview.editor_meta": "Editor  ·  {family}  ·  {size} pt  ·  {style}",
    "settings.preview.canvas": "Canvas",
    "settings.preview.panel": "Panel",
    "settings.preview.selected": "Selected",
    "settings.preview.meta": "{theme}  ·  {accent}  ·  {font}  ·  {size} pt  ·  {style}",
    "plugin.catalog.title": "Extension Catalog",
    "plugin.catalog.hint": "External entries are metadata only: untrusted and unloaded.",
    "plugin.catalog.empty": "No extension manifests found",
    "plugin.catalog.approve": "Record Approval",
    "plugin.catalog.revoke": "Revoke Approval",
    "plugin.catalog.status.valid": "valid",
    "plugin.catalog.status.invalid": "invalid",
    "plugin.catalog.status.incompatible": "incompatible",
    "plugin.catalog.status.duplicate": "duplicate",
    "plugin.catalog.trust.untrusted": "untrusted",
    "plugin.catalog.approval.approved": "approved",
    "plugin.catalog.approval.stale": "stale",
    "plugin.catalog.approval.not_approved": "not-approved",
    "plugin.catalog.execution.not_evaluated": "not-evaluated",
    "plugin.catalog.execution.denied": "denied",
    "plugin.catalog.execution.authorized": "authorized",
    "plugin.catalog.value.yes": "yes",
    "plugin.catalog.value.no": "no",
    "plugin.catalog.value.none": "none",
    "plugin.catalog.field.source": "Source",
    "plugin.catalog.field.status": "Status",
    "plugin.catalog.field.trust": "Trust",
    "plugin.catalog.field.approval": "Approval",
    "plugin.catalog.field.loadable": "Loadable",
    "plugin.catalog.field.execution": "Execution",
    "plugin.catalog.field.execution_reason": "Execution reason",
    "plugin.catalog.field.execution_requirements": "Execution requirements",
    "plugin.catalog.field.api": "API",
    "plugin.catalog.field.permissions": "Permissions",
    "plugin.catalog.field.entrypoint": "Entrypoint metadata",
    "plugin.catalog.field.descriptor": "Descriptor SHA-256",
    "plugin.catalog.field.reason": "Reason",
    "plugin.catalog.reason.not_evaluated": "not-evaluated",
    "plugin.catalog.reason.authorized": "authorized",
    "plugin.catalog.reason.invalid_evidence": "invalid-evidence",
    "plugin.catalog.reason.external_execution_disabled": "external-execution-disabled",
    "plugin.catalog.reason.catalog_entry_invalid": "catalog-entry-invalid",
    "plugin.catalog.reason.untrusted_plugin": "untrusted-plugin",
    "plugin.catalog.reason.approval_missing": "approval-missing",
    "plugin.catalog.reason.approval_stale": "approval-stale",
    "plugin.catalog.reason.signature_not_valid": "signature-not-valid",
    "plugin.catalog.reason.code_identity_not_verified": "code-identity-not-verified",
    "plugin.catalog.reason.plugin_not_enabled": "plugin-not-enabled",
    "plugin.catalog.reason.permissions_not_granted": "permissions-not-granted",
    "plugin.catalog.reason.host_containment_unavailable": "host-containment-unavailable",
    "plugin.catalog.reason.executor_unavailable": "executor-unavailable",
    "plugin.status.title": "Plugin Status",
    "plugin.status.empty": "No explicitly registered plugins",
    "plugin.status.summary": (
        "Only explicitly registered in-process plugins are shown; "
        "external catalog entries remain unloaded."
    ),
    "plugin.enable": "Enable",
    "plugin.disable": "Disable",
    "plugin.trusted": "trusted",
    "plugin.untrusted": "untrusted",
    "plugin.enabled": "enabled",
    "plugin.disabled": "disabled",
    "plugin.active": "active",
    "plugin.inactive": "inactive",
    "plugin.value.true": "true",
    "plugin.value.false": "false",
    "plugin.id": "Plugin ID",
    "plugin.version": "Version",
    "plugin.trust": "Trusted",
    "plugin.permissions": "Permissions",
    "plugin.none": "none",
    "plugin.error": "Error",
    "status.local": "LOCAL",
    "status.ready": "READY",
    "status.working": "WORKING",
    "status.attention": "ATTENTION",
    "status.error": "ERROR",
    "accessibility.shell_status": "QuillForge shell status",
    "accessibility.workspace_context": "Workspace context",
    "accessibility.shell_phase": "Shell phase",
    "accessibility.shell_phase_description": "QuillForge shell phase: {phase}",
    "accessibility.shell_notification": "Shell notification",
    "about.title": "About QuillForge",
    "about.body": "QuillForge\n\nA cute anime-forge text and code editor for creative work.",
    "recovery.title": "Recover unsaved work",
    "recovery.untitled": "Untitled document",
    "recovery.restore": "Restore",
    "recovery.discard": "Discard",
    "recovery.later": "Later",
    "recovery.available": "A recovery snapshot is available for:\n{path}",
    "recovery.source.untitled": "There is no original file; recovery will remain unsaved.",
    "recovery.source.unchanged": "The original file matches the snapshot baseline.",
    "recovery.source.changed": (
        "The original file changed; saving will require the existing conflict check."
    ),
    "recovery.source.missing": (
        "The original file is missing; recovery will remain unsaved until you choose a target."
    ),
    "recovery.source.unavailable": (
        "The original file could not be checked; recovery will remain unsaved until reviewed."
    ),
    "recovery.source.unknown": "The original file status is unknown.",
    "recovery.details": (
        "Snapshot time: {created}\n{source}\n\n"
        "Restore opens the snapshot as unsaved content and never writes the original file "
        "automatically."
    ),
    "dialog.save_before_close": "Save changes to {name} before closing?",
    "dialog.button.save": "Save",
    "dialog.button.discard": "Discard",
    "dialog.button.cancel": "Cancel",
    "dialog.button.ok": "OK",
    "dialog.open_document": "Open document",
    "dialog.save_document": "Save document",
    "dialog.open_workspace": "Open workspace folder",
    "dialog.text_filter": (
        "All files (*);;Text and source files (*.txt *.md *.py *.json *.csv *.yaml "
        "*.yml *.toml *.js *.ts *.html *.css *.xml *.ini *.log *.sql *.sh *.ps1)"
    ),
    "error.settings": "Settings failed",
    "error.operation": "Operation failed",
    "error.open": "Open failed",
    "error.save": "Save failed",
    "error.background": "Background operation in progress",
    "error.unsaved": "Unsaved changes",
    "error.wait_operation": "Wait for the current document operation to finish.",
    "error.wait_pending": (
        "Wait for {count} background operation or completion callback(s) to finish before quitting."
    ),
    "command.file.new": "&New",
    "command.file.open": "&Open...",
    "command.file.open-workspace": "Open &Workspace...",
    "command.file.save": "&Save",
    "command.file.save-as": "Save &As...",
    "command.file.close": "&Close Tab",
    "command.file.recover": "Recover Unsaved &Work...",
    "command.file.quit": "&Quit",
    "command.edit.undo": "&Undo",
    "command.edit.redo": "&Redo",
    "command.edit.cut": "Cu&t",
    "command.edit.copy": "&Copy",
    "command.edit.paste": "&Paste",
    "command.edit.select-all": "Select &All",
    "command.edit.find": "&Find",
    "command.edit.replace": "Find and &Replace",
    "command.edit.find-in-files": "Find in &Files...",
    "command.tools.command-palette": "Command &Palette",
    "command.tools.settings": "&Settings...",
    "command.tools.extension-catalog": "&Extension Catalog...",
    "command.tools.plugin-status": "&Plugin Status...",
    "command.tools.plugin-host-diagnostics": "Plugin Host &Diagnostics...",
    "command.tools.document-stats": "Document Statistics",
    "command.help.about": "&About QuillForge",
}

_CHINESE: dict[str, str] = {
    **_ENGLISH,
    "document.untitled": "未命名文档",
    "menu.file": "文件(&F)",
    "menu.edit": "编辑(&E)",
    "menu.tools": "工具(&T)",
    "menu.help": "帮助(&H)",
    "toolbar.command_rail": "✦ 快捷操作栏",
    "toolbar.new": "新建",
    "toolbar.open": "打开",
    "toolbar.save": "保存",
    "toolbar.find": "查找",
    "toolbar.replace": "替换",
    "toolbar.command_palette": "命令面板",
    "toolbar.workspace": "工作区",
    "toolbar.context": "✦ 本地工作区 · 默认安全",
    "workspace.dock": "✦ 资源管理器",
    "workspace.eyebrow": "项目文件",
    "workspace.tree": "工作区文件",
    "workspace.path": "当前工作区路径",
    "workspace.empty": "选择一个文件夹，开始浏览项目。",
    "workspace.empty_directory": "这个文件夹是空的。",
    "workspace.no_selection": "尚未选择工作区",
    "workspace.choose": "选择一个文件夹，开始浏览项目。",
    "workspace.open_folder": "打开项目文件夹",
    "workspace.open_file": "打开文件",
    "workspace.open_folder_hint": "选择文件夹以浏览项目文件。",
    "workspace.open_file_hint": "选择一个文件直接打开。",
    "workspace.up": "返回上级",
    "workspace.up_hint": "返回上一级文件夹。",
    "workspace.cancel": "取消",
    "workspace.cancel_hint": "取消当前工作区操作。",
    "workspace.loading": "正在加载工作区…",
    "workspace.entries": "{count} 个条目{suffix} · 单击文件 / 双击文件夹",
    "workspace.truncated_suffix": " · 列表已截断",
    "workspace.more_entries": "更多条目未显示（已达到上限）",
    "workspace.entry_file_hint": "文件 · 单击打开",
    "workspace.entry_directory_hint": "文件夹 · 双击进入",
    "workspace.entry_inaccessible_hint": "无法访问的条目",
    "find.find": "查找",
    "find.replace": "替换",
    "find.case": "区分大小写",
    "find.previous": "上一个",
    "find.next": "下一个",
    "find.replace_one": "替换",
    "find.replace_all": "全部替换",
    "find.cancel": "取消",
    "find.close": "关闭",
    "find.placeholder": "输入查找内容",
    "find.replace_placeholder": "输入替换内容",
    "find.status.operation": "另一个编辑器操作正在进行中",
    "find.status.enter": "请输入查找内容",
    "find.status.match": "已找到匹配项",
    "find.status.none": "没有找到匹配项",
    "find.status.first": "请先查找匹配项",
    "find.status.replaced_one": "已替换 1 个匹配项",
    "find.status.preparing": "正在准备全部替换…",
    "find.status.counting": "正在统计匹配项…",
    "find.status.cancelled_changed": "文档发生变化，全部替换已取消",
    "find.status.counting_progress": "正在统计匹配项… {count}",
    "find.status.replacing_progress": "正在替换匹配项… {count}",
    "find.status.failed_restored": "全部替换失败；文档已恢复",
    "find.status.failed_partial": "全部替换失败；文档可能包含部分修改",
    "find.status.limit": "已停止：匹配项超过 {limit} 个；文档未修改",
    "find.error.limit": "全部替换最多支持 {limit} 个匹配项。",
    "find.status.cancelled_unchanged": "全部替换已取消；文档未修改",
    "find.status.replaced_count": "已替换 {count} 个匹配项",
    "command_palette.title": "命令面板",
    "command_palette.placeholder": "按名称或 ID 搜索命令",
    "command_palette.hint": "回车执行 · Esc 关闭",
    "command_palette.results": "命令结果",
    "command_palette.empty.initial": "当前没有可用命令",
    "command_palette.empty.no_matches": "没有匹配当前搜索的命令",
    "search.title": "在文件中查找",
    "search.placeholder": "搜索文本",
    "search.case": "区分大小写",
    "search.search": "搜索",
    "search.cancel": "取消",
    "search.close": "关闭",
    "search.results": "工作区搜索结果",
    "search.initial": "输入文本以搜索当前工作区",
    "search.empty.initial": "输入搜索内容，开始查找当前工作区",
    "search.empty.loading": "正在搜索当前工作区…",
    "search.empty.no_matches": "当前工作区没有匹配项",
    "search.empty.cancelled": "搜索已取消；再次搜索以刷新结果",
    "search.empty.error": "暂时无法显示结果，请修正搜索内容后重试",
    "search.root": "搜索根目录：{root}",
    "search.loading": "正在搜索工作区…",
    "search.cancelling": "正在取消工作区搜索…",
    "search.enter": "请输入搜索文本",
    "search.failed": "工作区搜索失败：{message}",
    "search.cancelled": "工作区搜索已取消；已保留当前结果",
    "search.summary.complete": (
        "搜索完成：{files} 个文件中找到 {matches} 个匹配项；已扫描 {bytes} 字节{diagnostics}"
    ),
    "search.summary.cancelled": (
        "搜索已取消：{files} 个文件中找到 {matches} 个匹配项；已扫描 {bytes} 字节{diagnostics}"
    ),
    "search.summary.limited": (
        "搜索受限（{reason}）：{files} 个文件中找到 {matches} 个匹配项；"
        "已扫描 {bytes} 字节{diagnostics}"
    ),
    "search.summary.diagnostics": "（{count} 条诊断信息）",
    "search.summary.diagnostics_truncated": "（诊断信息已截断）",
    "search.diagnostics": "诊断信息",
    "search.diagnostics_count": "诊断信息（{count}）",
    "search.diagnostics_truncated": "诊断信息（已显示 {count} 条；更多记录已省略）",
    "search.more_diagnostics": "更多诊断信息因记录上限未显示。",
    "search.outside_workspace": "<位于所选工作区之外>",
    "settings.title": "QuillForge 设置",
    "settings.appearance_group": "外观",
    "settings.editor_group": "编辑器",
    "settings.language": "界面语言",
    "settings.language_english": "English",
    "settings.language_chinese": "简体中文",
    "settings.theme": "主题",
    "settings.theme_ink": "墨色 · 紫罗兰",
    "settings.theme_paper": "纸张 · 沙金",
    "settings.theme_sakura": "樱花 · 糖果",
    "settings.accent": "强调色",
    "settings.accent_violet": "紫罗兰",
    "settings.accent_cyan": "青色",
    "settings.accent_rose": "玫瑰红",
    "settings.accent_amber": "琥珀金",
    "settings.ui_font": "界面字体",
    "settings.ui_font_size": "界面字号",
    "settings.font_size_suffix": " 磅",
    "settings.ui_font_style": "界面样式",
    "settings.editor_font": "编辑器字体",
    "settings.editor_font_size": "编辑器字号",
    "settings.editor_font_style": "编辑器样式",
    "settings.font_style_regular": "常规",
    "settings.font_style_semibold": "半粗",
    "settings.font_style_bold": "粗体",
    "settings.font_style_italic": "斜体",
    "settings.wrap": "自动换行",
    "settings.line_numbers": "显示行号",
    "settings.motion": "启用平滑过渡动画",
    "settings.apply_note": "语言和主题保存后立即生效。",
    "settings.ok": "保存",
    "settings.cancel": "取消",
    "settings.restore_defaults": "恢复默认值",
    "settings.restore_defaults_hint": "只重置当前草稿；点击保存后才会保留。",
    "settings.draft.clean": "当前没有未保存的设置更改。",
    "settings.draft.changed": "有未保存的设置更改 · 点击保存后应用。",
    "settings.font_status": (
        "界面字体“{ui_font}”：{ui_status}；编辑器字体“{editor_font}”：" + "{editor_status}。"
    ),
    "settings.font_status_installed": "已安装",
    "settings.font_status_fallback": "未安装，将回退到系统字体",
    "settings.font_status_unknown": "暂时无法读取安装状态",
    "settings.unsupported_font_note": "系统未安装的字体会自动回退到默认字体。",
    "settings.preview.title": "实时预览",
    "settings.preview.accent": "强调色",
    "settings.preview.sample": "✦ QuillForge  ·  Aa  你好",
    "settings.preview.editor_sample": "def forge(): 你好",
    "settings.preview.editor_meta": "编辑器  ·  {family}  ·  {size} 磅  ·  {style}",
    "settings.preview.canvas": "画布",
    "settings.preview.panel": "面板",
    "settings.preview.selected": "选中",
    "settings.preview.meta": "{theme}  ·  {accent}  ·  {font}  ·  {size} 磅  ·  {style}",
    "plugin.catalog.title": "扩展目录",
    "plugin.catalog.hint": "外部条目仅作为元数据展示：不信任且不会加载。",
    "plugin.catalog.empty": "未找到扩展清单",
    "plugin.catalog.approve": "记录批准",
    "plugin.catalog.revoke": "撤销批准",
    "plugin.catalog.status.valid": "有效",
    "plugin.catalog.status.invalid": "无效",
    "plugin.catalog.status.incompatible": "不兼容",
    "plugin.catalog.status.duplicate": "重复",
    "plugin.catalog.trust.untrusted": "不信任",
    "plugin.catalog.approval.approved": "已批准",
    "plugin.catalog.approval.stale": "已过期",
    "plugin.catalog.approval.not_approved": "未批准",
    "plugin.catalog.execution.not_evaluated": "未评估",
    "plugin.catalog.execution.denied": "已拒绝",
    "plugin.catalog.execution.authorized": "已授权",
    "plugin.catalog.value.yes": "是",
    "plugin.catalog.value.no": "否",
    "plugin.catalog.value.none": "无",
    "plugin.catalog.field.source": "来源",
    "plugin.catalog.field.status": "状态",
    "plugin.catalog.field.trust": "信任状态",
    "plugin.catalog.field.approval": "批准状态",
    "plugin.catalog.field.loadable": "可加载",
    "plugin.catalog.field.execution": "执行",
    "plugin.catalog.field.execution_reason": "执行原因",
    "plugin.catalog.field.execution_requirements": "执行要求",
    "plugin.catalog.field.api": "API",
    "plugin.catalog.field.permissions": "权限",
    "plugin.catalog.field.entrypoint": "入口元数据",
    "plugin.catalog.field.descriptor": "描述信息 SHA-256",
    "plugin.catalog.field.reason": "原因",
    "plugin.catalog.reason.not_evaluated": "未评估",
    "plugin.catalog.reason.authorized": "已授权",
    "plugin.catalog.reason.invalid_evidence": "证据无效",
    "plugin.catalog.reason.external_execution_disabled": "外部执行已禁用",
    "plugin.catalog.reason.catalog_entry_invalid": "目录条目无效",
    "plugin.catalog.reason.untrusted_plugin": "插件不受信任",
    "plugin.catalog.reason.approval_missing": "缺少批准",
    "plugin.catalog.reason.approval_stale": "批准已过期",
    "plugin.catalog.reason.signature_not_valid": "签名无效",
    "plugin.catalog.reason.code_identity_not_verified": "代码身份未验证",
    "plugin.catalog.reason.plugin_not_enabled": "插件未启用",
    "plugin.catalog.reason.permissions_not_granted": "权限未授予",
    "plugin.catalog.reason.host_containment_unavailable": "宿主隔离不可用",
    "plugin.catalog.reason.executor_unavailable": "执行器不可用",
    "plugin.status.title": "插件状态",
    "plugin.status.empty": "没有显式注册的插件",
    "plugin.status.summary": "这里只显示显式注册的进程内插件；外部目录条目保持未加载。",
    "plugin.enable": "启用",
    "plugin.disable": "停用",
    "plugin.trusted": "已信任",
    "plugin.untrusted": "不信任",
    "plugin.enabled": "已启用",
    "plugin.disabled": "已停用",
    "plugin.active": "活动中",
    "plugin.inactive": "未活动",
    "plugin.value.true": "是",
    "plugin.value.false": "否",
    "plugin.id": "插件 ID",
    "plugin.version": "版本",
    "plugin.trust": "信任状态",
    "plugin.permissions": "权限",
    "plugin.none": "无",
    "plugin.error": "错误",
    "status.local": "本地",
    "status.ready": "就绪",
    "status.working": "处理中",
    "status.attention": "需注意",
    "status.error": "错误",
    "accessibility.shell_status": "QuillForge 界面状态",
    "accessibility.workspace_context": "工作区上下文",
    "accessibility.shell_phase": "界面阶段",
    "accessibility.shell_phase_description": "QuillForge 界面阶段：{phase}",
    "accessibility.shell_notification": "界面通知",
    "about.title": "关于 QuillForge",
    "about.body": "QuillForge\n\n一款为创作而生的可爱二次元文本与代码编辑器。",
    "recovery.title": "恢复未保存的工作",
    "recovery.untitled": "未命名文档",
    "recovery.restore": "恢复",
    "recovery.discard": "丢弃",
    "recovery.later": "稍后处理",
    "recovery.available": "发现以下文档的恢复快照：\n{path}",
    "recovery.source.untitled": "没有原始文件；恢复内容将保持为未保存文档。",
    "recovery.source.unchanged": "原始文件与快照基线一致。",
    "recovery.source.changed": "原始文件已发生变化；保存时仍会执行现有冲突检查。",
    "recovery.source.missing": "原始文件不存在；请选择目标后才能保存恢复内容。",
    "recovery.source.unavailable": "无法检查原始文件；恢复内容将保持未保存，等待后续确认。",
    "recovery.source.unknown": "原始文件状态未知。",
    "recovery.details": (
        "快照时间：{created}\n{source}\n\n恢复操作会以未保存内容打开快照，不会自动写入原始文件。"
    ),
    "dialog.save_before_close": "关闭前保存对 {name} 的修改吗？",
    "dialog.button.save": "保存",
    "dialog.button.discard": "丢弃",
    "dialog.button.cancel": "取消",
    "dialog.button.ok": "确定",
    "dialog.open_document": "打开文档",
    "dialog.save_document": "保存文档",
    "dialog.open_workspace": "打开工作区文件夹",
    "dialog.text_filter": (
        "所有文件 (*);;文本与源代码文件 (*.txt *.md *.py *.json *.csv *.yaml "
        "*.yml *.toml *.js *.ts *.html *.css *.xml *.ini *.log *.sql *.sh *.ps1)"
    ),
    "error.settings": "设置失败",
    "error.operation": "操作失败",
    "error.open": "打开失败",
    "error.save": "保存失败",
    "error.background": "后台操作正在进行",
    "error.unsaved": "存在未保存修改",
    "error.wait_operation": "请等待当前文档操作完成。",
    "error.wait_pending": "退出前请等待 {count} 个后台操作或完成回调结束。",
    "command.file.new": "新建",
    "command.file.open": "打开…",
    "command.file.open-workspace": "打开工作区…",
    "command.file.save": "保存",
    "command.file.save-as": "另存为…",
    "command.file.close": "关闭标签页",
    "command.file.recover": "恢复未保存的工作…",
    "command.file.quit": "退出",
    "command.edit.undo": "撤销",
    "command.edit.redo": "重做",
    "command.edit.cut": "剪切",
    "command.edit.copy": "复制",
    "command.edit.paste": "粘贴",
    "command.edit.select-all": "全选",
    "command.edit.find": "查找",
    "command.edit.replace": "查找并替换",
    "command.edit.find-in-files": "在文件中查找…",
    "command.tools.command-palette": "命令面板",
    "command.tools.settings": "设置…",
    "command.tools.extension-catalog": "扩展目录…",
    "command.tools.plugin-status": "插件状态…",
    "command.tools.plugin-host-diagnostics": "插件宿主诊断…",
    "command.tools.document-stats": "文档统计",
    "command.help.about": "关于 QuillForge",
}

_BUNDLES: Mapping[Locale, Mapping[str, str]] = {
    "en-US": _ENGLISH,
    "zh-CN": _CHINESE,
}


def normalize_locale(value: object) -> Locale:
    """Return a supported locale without allowing malformed persisted values."""
    if isinstance(value, str) and value in _BUNDLES:
        return cast(Locale, value)
    return DEFAULT_LOCALE


def tr(key: str, locale: Locale = DEFAULT_LOCALE, **values: object) -> str:
    """Resolve one bounded UI string and format only its named values."""
    text = _BUNDLES[normalize_locale(locale)].get(key, _ENGLISH.get(key, key))
    return text.format(**values) if values else text


def catalog_value(category: str, value: str, locale: Locale = DEFAULT_LOCALE) -> str:
    """Translate a known catalog enum while preserving future raw values."""
    key = f"plugin.catalog.{category}.{value.replace('-', '_')}"
    localized = tr(key, locale)
    return value if localized == key else localized


def command_title(command_id: str, fallback: str, locale: Locale = DEFAULT_LOCALE) -> str:
    """Translate a core command while preserving plugin-provided fallback titles."""
    key = f"command.{command_id}"
    return tr(key, locale) if key in _ENGLISH else fallback


def plugin_display_name(plugin_id: str, fallback: str, locale: Locale = DEFAULT_LOCALE) -> str:
    """Translate one built-in plugin name while preserving external metadata."""
    if plugin_id != "quillforge.document-stats":
        return fallback
    return tr("command.tools.document-stats", locale)


def workspace_search_summary(
    *,
    cancelled: bool,
    truncated: bool,
    limit_reason: str | None,
    matches: int,
    files_scanned: int,
    bytes_scanned: int,
    diagnostic_count: int,
    diagnostics_truncated: bool,
    locale: Locale = DEFAULT_LOCALE,
) -> str:
    """Format typed search counters at the presentation boundary."""
    reason_labels = {
        "max-files": "file limit",
        "max-bytes": "byte limit",
        "max-total-bytes": "total byte limit",
        "max-results": "result limit",
        "max-depth": "depth limit",
        "max-line-bytes": "line length limit",
        "max-issue-records": "diagnostic limit",
    }
    reason = (
        reason_labels.get(limit_reason, limit_reason or "limit")
        if normalize_locale(locale) == "en-US"
        else {
            "max-files": "文件数上限",
            "max-bytes": "文件字节上限",
            "max-total-bytes": "总字节上限",
            "max-results": "结果数上限",
            "max-depth": "目录深度上限",
            "max-line-bytes": "行长度上限",
            "max-issue-records": "诊断记录上限",
        }.get(limit_reason, limit_reason or "上限")
    )
    diagnostics = (
        tr("search.summary.diagnostics", locale, count=diagnostic_count) if diagnostic_count else ""
    )
    if diagnostics_truncated:
        diagnostics += tr("search.summary.diagnostics_truncated", locale)
    key = "search.summary.cancelled" if cancelled else "search.summary.complete"
    if truncated:
        key = "search.summary.limited"
    return tr(
        key,
        locale,
        reason=reason,
        matches=f"{matches:,}",
        files=f"{files_scanned:,}",
        bytes=f"{bytes_scanned:,}",
        diagnostics=diagnostics,
    )


_PLUGIN_FAILURE = re.compile(
    r"^Plugin (?P<plugin>.+) failed during (?P<phase>[^:]+): (?P<detail>.*)$"
)
_PLUGIN_FAILURE_PHASES = {
    "activate": "激活",
    "deactivate": "停用",
    "command": "命令处理",
    "event": "事件处理",
}
_PLUGIN_CATALOG_SUMMARY = re.compile(
    r"^Extension catalog: (?P<valid>\d+) valid, "
    r"(?P<incompatible>\d+) incompatible, "
    r"(?P<invalid>\d+) invalid, "
    r"(?P<duplicate>\d+) duplicate"
    r"(?P<catalog_suffix> \(catalog limit reached\)| \(scan issue: (?P<scan_error>.*?)\))?; "
    r"external entries remain untrusted and unloaded"
    r"; approvals (?P<approved>\d+) approved, "
    r"(?P<stale>\d+) stale, "
    r"(?P<not_approved>\d+) not-approved"
    r"(?P<ledger> \(ledger issue: (?P<ledger_error>.*?)\))?"
    r"; external execution (?P<denied>\d+) denied, "
    r"(?P<authorized>\d+) authorized$"
)
_PLUGIN_HOST_SUMMARY = re.compile(
    r"^Plugin host (?P<state>[a-z-]+)"
    r"(?P<launcher>; launcher PID \d+)?"
    r"(?P<reported>; reported host PID \d+)?; "
    r"containment (?P<containment>[a-z-]+)"
    r"(?P<containment_detail> \([^)]*\))?"
    r"(?P<limits> \[[^]]*\])?; external execution "
    r"(?P<execution>enabled|disabled): (?P<detail>.*)$"
)
_FIND_REPLACED_COUNT = re.compile(r"^Replaced (?P<count>[\d,]+) matches$")
_FIND_LIMIT = re.compile(r"^Stopped: more than (?P<limit>[\d,]+) matches; document unchanged$")
_FIND_LIMIT_ERROR = re.compile(r"^Replace All is limited to (?P<limit>[\d,]+) matches\.$")
_DOCUMENT_STATS = re.compile(
    r"^Document statistics: (?P<lines>[\d,]+) lines, "
    r"(?P<characters>[\d,]+) characters$"
)
_OPEN_PATH_ERROR = re.compile(r"^Cannot open path: (?P<path>.+) \((?P<detail>.*)\)$")
_AUTOSAVE_ERROR = re.compile(
    r"^(?P<kind>Autosave capture setup failed|Autosave capture failed|Autosave failed) "
    r"for (?P<name>.+): (?P<detail>.*)$"
)
_CLOSE_PENDING = re.compile(
    r"^Wait for (?P<count>[\d,]+) background operation or completion callback\(s\) "
    r"to finish before quitting\.$"
)

_RECOVERED_MESSAGE_PREFIX = "Recovered "
_RECOVERED_MESSAGE_SUFFIX = "; content remains unsaved"


def _localize_plugin_failure(message: str, locale: Locale) -> str | None:
    match = _PLUGIN_FAILURE.fullmatch(message)
    if match is None:
        return None
    phase = _PLUGIN_FAILURE_PHASES.get(match["phase"], match["phase"])
    detail = localize_message(match["detail"], locale)
    return f"插件 {match['plugin']} 在{phase}阶段失败：{detail}"


def _localize_nested_error(message: str, locale: Locale) -> str | None:
    """Translate a known presentation wrapper and its application error detail."""
    wrappers = (
        ("Workspace operation failed: ", "工作区操作失败："),
        ("Workspace search failed: ", "工作区搜索失败："),
        ("Descriptor governance failed: ", "描述信息治理失败："),
        ("Extension catalog scan failed: ", "扩展目录扫描失败："),
        ("Plugin host diagnostic failed: ", "插件宿主诊断失败："),
        ("Plugin lifecycle change failed: ", "插件生命周期变更失败："),
        ("Recovery cleanup failed: ", "恢复清理失败："),
        ("Recovery scan failed: ", "恢复扫描失败："),
    )
    for prefix, localized_prefix in wrappers:
        if message.startswith(prefix):
            detail = localize_message(message[len(prefix) :], locale)
            return localized_prefix + detail

    open_path = _OPEN_PATH_ERROR.fullmatch(message)
    if open_path is not None:
        detail = localize_message(open_path["detail"], locale)
        return f"无法打开路径：{open_path['path']}（{detail}）"

    autosave = _AUTOSAVE_ERROR.fullmatch(message)
    if autosave is not None:
        kind = {
            "Autosave capture setup failed": "自动保存捕获初始化失败",
            "Autosave capture failed": "自动保存捕获失败",
            "Autosave failed": "自动保存失败",
        }[autosave["kind"]]
        detail = localize_message(autosave["detail"], locale)
        return f"{kind}：{autosave['name']}：{detail}"
    return None


def _localize_plugin_catalog_summary(message: str) -> str | None:
    match = _PLUGIN_CATALOG_SUMMARY.fullmatch(message)
    if match is None:
        return None
    catalog_suffix = ""
    if match["catalog_suffix"] == " (catalog limit reached)":
        catalog_suffix = "（已达到目录上限）"
    elif match["scan_error"] is not None:
        catalog_suffix = f"（扫描问题：{match['scan_error']}）"
    ledger_suffix = (
        f"（清单问题：{match['ledger_error']}）" if match["ledger_error"] is not None else ""
    )
    return (
        f"扩展目录：{match['valid']} 个有效，"
        f"{match['incompatible']} 个不兼容，"
        f"{match['invalid']} 个无效，"
        f"{match['duplicate']} 个重复{catalog_suffix}；"
        "外部条目仍不受信任且未加载；"
        f"批准 {match['approved']} 个已批准，"
        f"{match['stale']} 个已过期，"
        f"{match['not_approved']} 个未批准{ledger_suffix}；"
        f"外部执行 {match['denied']} 个被拒绝，"
        f"{match['authorized']} 个已授权"
    )


def _localize_plugin_host_summary(message: str) -> str | None:
    match = _PLUGIN_HOST_SUMMARY.fullmatch(message)
    if match is None:
        return None
    states = {
        "ready": "就绪",
        "rejected": "已拒绝",
        "timeout": "超时",
        "crashed": "崩溃",
        "protocol-error": "协议错误",
        "containment-error": "隔离错误",
    }
    containment = {
        "attached": "已附加",
        "attached-after-start": "启动后附加",
        "unsupported": "不支持",
        "failed": "失败",
        "not-requested": "未请求",
    }
    launcher = (
        match["launcher"].replace("; launcher PID ", "；启动器 PID ") if match["launcher"] else ""
    )
    reported = (
        match["reported"].replace("; reported host PID ", "；报告的宿主 PID ")
        if match["reported"]
        else ""
    )
    detail = match["containment_detail"] or ""
    limits = match["limits"] or ""
    execution = "已启用" if match["execution"] == "enabled" else "已禁用"
    return (
        f"插件宿主 {states.get(match['state'], match['state'])}{launcher}{reported}；"
        f"隔离 {containment.get(match['containment'], match['containment'])}"
        f"{detail}{limits}；外部执行{execution}：{match['detail']}"
    )


def _localize_find_status(message: str, locale: Locale) -> str | None:
    """Translate count-bearing Find/Replace outcomes without partial English tails."""
    replaced = _FIND_REPLACED_COUNT.fullmatch(message)
    if replaced is not None:
        return tr("find.status.replaced_count", locale, count=replaced["count"])
    limit = _FIND_LIMIT.fullmatch(message)
    if limit is not None:
        return tr("find.status.limit", locale, limit=limit["limit"])
    limit_error = _FIND_LIMIT_ERROR.fullmatch(message)
    if limit_error is not None:
        return tr("find.error.limit", locale, limit=limit_error["limit"])
    return None


def _localize_document_stats(message: str, locale: Locale) -> str | None:
    """Translate the built-in document-statistics notification counters."""
    match = _DOCUMENT_STATS.fullmatch(message)
    if match is None:
        return None
    return f"文档统计：{match['lines']} 行，{match['characters']} 个字符"


def _localize_close_guard_message(message: str, locale: Locale) -> str | None:
    """Translate the bounded pending-work close guard without partial tails."""
    match = _CLOSE_PENDING.fullmatch(message)
    if match is None:
        return None
    return tr("error.wait_pending", locale, count=match["count"])


def _localize_decode_reason(reason: str) -> str:
    """Translate stable Python codec reasons without hiding unknown details."""
    return {
        "invalid start byte": "无效的起始字节",
        "invalid continuation byte": "无效的连续字节",
        "unexpected end of data": "数据意外结束",
        "truncated data": "数据被截断",
    }.get(reason, reason)


def _localize_os_error(error: OSError) -> str | None:
    """Return a path-preserving Chinese summary for common file failures."""
    filename = getattr(error, "filename", None)
    target = str(filename) if filename is not None else str(error)
    if isinstance(error, FileNotFoundError):
        return f"找不到文件或目录：{target}"
    if isinstance(error, PermissionError):
        return f"没有权限访问：{target}"
    if isinstance(error, IsADirectoryError):
        return f"目标是文件夹，不能作为文件打开：{target}"
    if isinstance(error, NotADirectoryError):
        return f"路径中的目录不存在：{target}"
    return None


def localize_exception(error: Exception, locale: Locale = DEFAULT_LOCALE) -> str:
    """Localize typed I/O/codec failures while preserving useful diagnostics."""
    normalized = normalize_locale(locale)
    if normalized == "en-US":
        return str(error)
    if isinstance(error, (UnicodeDecodeError, UnicodeEncodeError)):
        action = "编码" if isinstance(error, UnicodeEncodeError) else "解码"
        return (
            f"无法使用 {error.encoding} {action}文本（位置 {error.start}）："
            f"{_localize_decode_reason(error.reason)}"
        )
    if isinstance(error, OSError):
        localized = _localize_os_error(error)
        if localized is not None:
            return localized
    return localize_message(str(error), normalized)


def localize_message(message: str, locale: Locale = DEFAULT_LOCALE) -> str:
    """Translate common shell notifications while preserving diagnostic details."""
    if normalize_locale(locale) == "en-US":
        return message
    exact = {
        "New document": "新建文档",
        "Settings saved": "设置已保存",
        "Settings failed": "设置失败",
        "Operation failed": "操作失败",
        "Operation in progress": "操作进行中",
        "Open failed": "打开失败",
        "Save failed": "保存失败",
        "No active document": "没有活动文档",
        "Replace All failed": "全部替换失败",
        "Text capture is not complete": "文本捕获尚未完成",
        "Replace All failed and rollback could not be completed": ("全部替换失败，且无法完成回滚"),
        "Replace All cancellation could not be rolled back": ("全部替换取消失败，且无法回滚"),
        "Background operation in progress": "后台操作正在进行",
        "Unsaved changes": "存在未保存修改",
        "A target path is required to save an untitled document": (
            "保存未命名文档前需要选择目标路径"
        ),
        "Select a workspace folder first": "请先选择工作区文件夹",
        "workspace search text must not be empty": "工作区搜索内容不能为空",
        "workspace search text is too long": "工作区搜索内容过长",
        "Only valid compatible descriptors may be approved": ("只能批准有效且兼容的描述信息"),
        "Descriptor identity and digest are required for approval": (
            "批准描述信息需要身份标识和摘要"
        ),
        "Cannot revoke an entry without a plugin ID": "没有插件 ID 不能撤销条目",
        "Session service is unavailable": "会话服务不可用",
        "Workspace operation failed: invalid workspace result": "工作区操作失败：无效的工作区结果",
        "Workspace operation failed: invalid directory result": "工作区操作失败：无效的目录结果",
        "That file is already open in another tab.": "该文件已在其他标签页中打开。",
        "The document service returned an invalid result.": "文档服务返回了无效结果。",
        "The settings service returned an invalid result.": "设置服务返回了无效结果。",
        "Wait for the current document operation to finish.": "请等待当前文档操作完成。",
        "Wait for the current document operation to finish": "请等待当前文档操作完成",
        "Save or close modified tabs before quitting.": "退出前请保存或关闭已修改的标签页。",
        "Wait for the workspace search to cancel before quitting.": (
            "请等待工作区搜索取消后再退出。"
        ),
        "Wait for the current background operation to finish before quitting.": (
            "请等待当前后台操作完成后再退出。"
        ),
        "The workspace service returned an invalid result": "工作区服务返回了无效结果",
        "The workspace service returned an invalid directory": "工作区服务返回了无效目录",
        "The search service returned an invalid result": "搜索服务返回了无效结果",
        "Workspace loading cancelled; current view kept": "工作区加载已取消；已保留当前视图",
        "That file is outside the selected workspace": "该文件位于所选工作区之外",
        "That search result is outside the selected workspace": "该搜索结果位于所选工作区之外",
        "Session document returned an invalid result; continuing": (
            "会话文档返回了无效结果；继续处理"
        ),
        "Previous session could not be read; original manifest was kept": (
            "无法读取上次会话；已保留原始清单"
        ),
        "Session state returned an invalid result": "会话状态返回了无效结果",
        "Session state could not be saved; the previous manifest was kept": (
            "无法保存会话状态；已保留上次清单"
        ),
        "Recovery snapshot kept; newer edits will be captured next cycle": (
            "已保留恢复快照；下一轮将捕获更新后的编辑内容"
        ),
        "Recovery snapshot kept for the next review": "已保留恢复快照，等待下次查看",
        "Recovery postponed because the original file is already open": (
            "原始文件已在打开状态，恢复操作已推迟"
        ),
        "Recovery snapshot discarded; the original file was not changed": (
            "已丢弃恢复快照；原始文件未被修改"
        ),
        "The recovery service returned an invalid inventory": "恢复服务返回了无效清单",
        "Extension approval governance is unavailable": "扩展批准治理不可用",
        "Extension catalog returned an invalid approval target": "扩展目录返回了无效批准目标",
        "Extension catalog operation already in progress": "扩展目录操作正在进行中",
        "Extension catalog returned an invalid result": "扩展目录返回了无效结果",
        "Plugin host returned an invalid diagnostic result": "插件宿主返回了无效诊断结果",
        "Plugin disabled by enablement policy": "插件已根据启用策略停用",
        "Only trusted plugins may be enabled": "只有受信任插件可以启用",
        "Invalid entrypoint metadata": "入口元数据无效",
        "Permissions must be a JSON array": "权限必须是 JSON 数组",
        "Plugin permissions must be a tuple": "插件权限必须是元组",
        "Plugin permissions must contain strings": "插件权限必须包含字符串",
        "Plugin permissions must not contain duplicates": "插件权限不能包含重复项",
        "Symbolic links are not traversed": "不会遍历符号链接",
        "Entry type is not supported": "不支持此条目类型",
        "symbolic link or reparse point skipped": "已跳过符号链接或重解析点",
        "excluded directory": "已排除目录",
        "maximum search depth reached": "已达到最大搜索深度",
        "unsupported filesystem entry": "不支持的文件系统条目",
        "regular files only": "仅支持普通文件",
        "file exceeds the configured per-file byte limit": "文件超过配置的单文件字节上限",
        "binary file skipped": "已跳过二进制文件",
        "invalid UTF-8 decoded with replacement": "无效 UTF-8 已使用替换字符解码",
        "Recording descriptor approval...": "正在记录描述信息批准…",
        "Recording descriptor revocation...": "正在记录描述信息撤销…",
        "Another editor operation is in progress": "另一个编辑器操作正在进行中",
        "Enter text to find": "请输入查找内容",
        "Match found": "已找到匹配项",
        "No matches": "没有找到匹配项",
        "Find a match first": "请先查找匹配项",
        "Replaced 1 match": "已替换 1 个匹配项",
        "Preparing Replace All...": "正在准备全部替换…",
        "Counting matches...": "正在统计匹配项…",
        "Replace All cancelled": "全部替换已取消",
        "Replace All cancelled because the document changed": "文档发生变化，全部替换已取消",
        "Replace All failed; document was restored": "全部替换失败；文档已恢复",
        "Replace All failed; document may contain partial changes": (
            "全部替换失败；文档可能包含部分修改"
        ),
        "Replace All cancelled; document unchanged": "全部替换已取消；文档未修改",
        "Restoring the previous session...": "正在恢复上次会话…",
        "Workspace navigation is unavailable": "工作区导航不可用",
        "Workspace search is unavailable": "工作区搜索不可用",
        "Open a workspace before searching files": "请先打开工作区再搜索文件",
        "Workspace loading cancelled": "工作区加载已取消",
        "Settings persistence is unavailable": "设置持久化不可用",
        "Settings save already in progress": "设置保存正在进行中",
        "No recovery snapshots found": "没有找到恢复快照",
        "Recovery is unavailable": "恢复功能不可用",
        "Recovery scan already in progress": "恢复扫描正在进行中",
        "Scanning extension catalog...": "正在扫描扩展目录…",
        "Extension catalog is unavailable": "扩展目录不可用",
        "Extension catalog scan already in progress": "扩展目录扫描正在进行中",
        "Plugin runtime control is unavailable": "插件运行时控制不可用",
        "Plugin host diagnostics are unavailable": "插件宿主诊断不可用",
        "Plugin host diagnostic already in progress": "插件宿主诊断正在进行中",
        "Starting isolated plugin host diagnostic...": "正在启动隔离插件宿主诊断…",
        "Descriptor governance updated; external code remains unloaded": (
            "描述信息治理已更新；外部代码仍未加载"
        ),
    }
    if message in exact:
        return exact[message]
    nested_error = _localize_nested_error(message, normalize_locale(locale))
    if nested_error is not None:
        return nested_error
    document_stats = _localize_document_stats(message, normalize_locale(locale))
    if document_stats is not None:
        return document_stats
    close_guard = _localize_close_guard_message(message, normalize_locale(locale))
    if close_guard is not None:
        return close_guard
    find_status = _localize_find_status(message, normalize_locale(locale))
    if find_status is not None:
        return find_status
    if message.startswith(_RECOVERED_MESSAGE_PREFIX) and message.endswith(
        _RECOVERED_MESSAGE_SUFFIX
    ):
        document_name = message[len(_RECOVERED_MESSAGE_PREFIX) : -len(_RECOVERED_MESSAGE_SUFFIX)]
        return f"已恢复 {document_name}；内容仍未保存"
    prefixes = (
        ("Cannot open path: ", "无法打开路径："),
        ("Unable to open path: ", "无法打开路径："),
        ("Workspace folder does not exist: ", "工作区文件夹不存在："),
        ("Workspace directory does not exist: ", "工作区目录不存在："),
        ("Path is outside the selected workspace: ", "路径位于所选工作区之外："),
        ("Workspace search root does not exist: ", "工作区搜索根目录不存在："),
        (
            "Approval ledger is unavailable; refusing mutation: ",
            "批准清单不可用；拒绝修改：",
        ),
        ("The document changed outside QuillForge: ", "文档在 QuillForge 外部发生了变化："),
        ("Opening ", "正在打开 "),
        ("Saving ", "正在保存 "),
        ("Loading workspace ", "正在加载工作区 "),
        ("Loading ", "正在加载 "),
        ("Opened ", "已打开 "),
        ("Saved ", "已保存 "),
        ("Counting matches... ", "正在统计匹配项… "),
        ("Replacing matches... ", "正在替换匹配项… "),
        ("Workspace opened: ", "工作区已打开："),
        ("Workspace search failed: ", "工作区搜索失败："),
        ("Workspace operation failed: ", "工作区操作失败："),
        ("directory unavailable: ", "目录不可用："),
        ("entry unavailable: ", "条目不可用："),
        ("file unavailable: ", "文件不可用："),
        ("file read failed: ", "文件读取失败："),
        ("Session document skipped: ", "已跳过会话文档："),
        ("Recovery scan failed: ", "恢复扫描失败："),
        ("Session document deferred for recovery review: ", "会话文档已推迟，等待恢复审核："),
        ("Autosave capture setup failed for ", "自动保存捕获初始化失败："),
        ("Autosave capture failed for ", "自动保存捕获失败："),
        ("Autosave failed for ", "自动保存失败："),
        ("Recovery cleanup failed: ", "恢复清理失败："),
        ("Descriptor governance failed: ", "描述信息治理失败："),
        ("Extension catalog scan failed: ", "扩展目录扫描失败："),
        ("Recovered ", "已恢复 "),
        ("Plugin host diagnostic failed: ", "插件宿主诊断失败："),
        ("Plugin lifecycle change failed: ", "插件生命周期变更失败："),
        ("Invalid plugin ID: ", "插件 ID 无效："),
        ("Invalid plugin manifest field: ", "插件清单字段无效："),
        ("Unsupported plugin permissions: ", "不支持的插件权限："),
        ("Already open: ", "已打开："),
        ("Plugin ", "插件 "),
        ("Command is no longer available: ", "命令已不可用："),
    )
    for localizer in (
        lambda value: _localize_plugin_failure(value, normalize_locale(locale)),
        _localize_plugin_catalog_summary,
        _localize_plugin_host_summary,
    ):
        localized = localizer(message)
        if localized is not None:
            return localized
    if message.startswith("Plugin "):
        for suffix, localized_suffix in (
            (" enabled", " 已启用"),
            (" disabled", " 已停用"),
            (" active", " 活动中"),
            (" inactive", " 未活动"),
        ):
            if message.endswith(suffix):
                return f"插件 {message[7 : -len(suffix)]}{localized_suffix}"
    for prefix, localized_prefix in prefixes:
        if message.startswith(prefix):
            return localized_prefix + message[len(prefix) :]
    return message
