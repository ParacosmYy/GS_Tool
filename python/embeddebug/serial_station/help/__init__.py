"""EmbedDebug 自包含帮助系统。"""

from embeddebug.serial_station.help.system import HelpSystem
from embeddebug.serial_station.help.topics import PROTOCOL_HELP, SHORTCUT_HELP, HelpEntry, HelpTopic

__all__ = ["HelpEntry", "HelpSystem", "HelpTopic", "PROTOCOL_HELP", "SHORTCUT_HELP"]
