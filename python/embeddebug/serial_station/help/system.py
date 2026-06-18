"""帮助系统: 按主题查询、全文检索与自定义条目注册。"""

from __future__ import annotations

from embeddebug.serial_station.help.topics import PROTOCOL_HELP, SHORTCUT_HELP, HelpEntry, HelpTopic

APP_NAME = "EmbedDebug"
APP_VERSION = "0.1.0"
APP_DESCRIPTION = "EmbedDebug 是基于 PyQt6 的嵌入式调试上位机。"
GITHUB_URL = "https://github.com/ParacosmYy/GS_Tool.git"
APP_LICENSE = "MIT"


def _build_default_entries() -> dict[HelpTopic, list[HelpEntry]]:
    entries: dict[HelpTopic, list[HelpEntry]] = {topic: [] for topic in HelpTopic}
    entries[HelpTopic.GETTING_STARTED].append(HelpEntry(HelpTopic.GETTING_STARTED, "快速开始", "1. 选择串口并连接\n2. 选择协议\n3. 输入命令按 Ctrl+Enter 发送", ["入门", "串口"]))
    entries[HelpTopic.PROTOCOLS].extend(PROTOCOL_HELP.values())
    entries[HelpTopic.SHORTCUTS].append(HelpEntry(HelpTopic.SHORTCUTS, "键盘快捷键", "\n".join(f"{i['key']}: {i['action']}" for i in SHORTCUT_HELP), ["快捷键"]))
    entries[HelpTopic.TROUBLESHOOTING].append(HelpEntry(HelpTopic.TROUBLESHOOTING, "串口连接失败", "确认设备上电、端口未占用、波特率一致。", ["串口", "失败"]))
    entries[HelpTopic.ABOUT].append(HelpEntry(HelpTopic.ABOUT, "关于 EmbedDebug", f"{APP_NAME} {APP_VERSION}\n{APP_DESCRIPTION}\n仓库: {GITHUB_URL}\n许可证: {APP_LICENSE}", ["关于", "version"]))
    return entries


class HelpSystem:
    """自包含帮助系统。"""

    def __init__(self) -> None:
        self._entries = _build_default_entries()

    def get_topic(self, topic: HelpTopic) -> list[HelpEntry]:
        return list(self._entries.get(topic, []))

    def search(self, query: str) -> list[HelpEntry]:
        text = query.strip().lower()
        if not text:
            return []
        results: list[HelpEntry] = []
        for entries in self._entries.values():
            for entry in entries:
                if text in entry.title.lower() or text in entry.content.lower() or any(text in t.lower() for t in entry.tags):
                    if entry not in results:
                        results.append(entry)
        return results

    def all_topics(self) -> dict[HelpTopic, list[HelpEntry]]:
        return {t: list(e) for t, e in self._entries.items()}

    def about_info(self) -> dict[str, str]:
        return {"app_name": APP_NAME, "version": APP_VERSION, "description": APP_DESCRIPTION, "github_url": GITHUB_URL, "license": APP_LICENSE}

    def register_entry(self, entry: HelpEntry) -> None:
        self._entries.setdefault(entry.topic, []).append(entry)
