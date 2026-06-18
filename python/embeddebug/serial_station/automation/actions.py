"""自动化动作定义与执行。"""

from __future__ import annotations

from collections.abc import Callable
from dataclasses import dataclass
from enum import Enum, auto

SendCallable = Callable[[str], None]
ActionContext = dict[str, list[str]]


class ActionType(Enum):
    """动作类型。"""

    SEND_COMMAND = auto()
    PLAY_SOUND = auto()
    LOG = auto()
    SET_LED = auto()
    DELAY = auto()


@dataclass
class AutomationAction:
    """单条动作配置。"""

    action_type: ActionType
    command: str = ""
    sound_path: str = ""
    log_message: str = ""
    led_state: str = ""
    delay_ms: int = 0

    def execute(
        self,
        send_callable: SendCallable,
        context: ActionContext | None = None,
    ) -> int:
        """执行动作。返回需要等待的毫秒数（仅 DELAY 返回 >0，其余返回 0）。"""
        ctx = context if context is not None else {}
        atype = self.action_type
        if atype is ActionType.SEND_COMMAND:
            if self.command:
                send_callable(self.command)
            ctx.setdefault("sent", []).append(self.command)
            return 0
        if atype is ActionType.PLAY_SOUND:
            ctx.setdefault("sounds", []).append(self.sound_path)
            return 0
        if atype is ActionType.LOG:
            ctx.setdefault("logs", []).append(self.log_message)
            return 0
        if atype is ActionType.SET_LED:
            ctx.setdefault("leds", []).append(self.led_state)
            return 0
        if atype is ActionType.DELAY:
            wait = max(0, self.delay_ms)
            ctx.setdefault("delays", []).append(str(wait))
            return wait
        return 0

    def to_dict(self) -> dict[str, object]:
        return {
            "action_type": self.action_type.name,
            "command": self.command,
            "sound_path": self.sound_path,
            "log_message": self.log_message,
            "led_state": self.led_state,
            "delay_ms": self.delay_ms,
        }

    @classmethod
    def from_dict(cls, data: dict[str, object]) -> "AutomationAction":
        return cls(
            action_type=ActionType[str(data["action_type"])],  # type: ignore[index]
            command=str(data.get("command", "")),
            sound_path=str(data.get("sound_path", "")),
            log_message=str(data.get("log_message", "")),
            led_state=str(data.get("led_state", "")),
            delay_ms=int(data.get("delay_ms", 0)),
        )
