"""Windows desktop entry point for the UART-first SerialForge shell."""

from __future__ import annotations

import sys
from collections.abc import Sequence

from ..composition import create_application, create_main_window
from ..domain.errors import SerialForgeError, TransportDependencyError
from .font_runtime import configure_application_font


def main(argv: Sequence[str] | None = None) -> int:
    """Start the Qt shell; all concrete devices are created by the composition root."""

    try:
        from .qt import QApplication

        app = QApplication(list(argv) if argv is not None else sys.argv)
        configure_application_font(app)
        context = create_application()
        window = create_main_window(context)
    except TransportDependencyError as exc:
        print(f"SerialForge 无法启动：{exc}", file=sys.stderr)
        return 2

    window.show()
    exit_code = app.exec()
    try:
        context.commands.shutdown(timeout=3.0)
    except SerialForgeError as exc:
        print(f"SerialForge 关闭时批量命令仍有任务未完成：{exc}", file=sys.stderr)
    try:
        context.session.shutdown(timeout=3.0)
    except SerialForgeError as exc:
        print(f"SerialForge 关闭时仍有 worker 未完成：{exc}", file=sys.stderr)
    try:
        context.replay.shutdown(timeout=3.0)
    except SerialForgeError as exc:
        print(f"SerialForge 关闭时历史回放仍有任务未完成：{exc}", file=sys.stderr)
    try:
        context.protocol.shutdown(timeout=3.0)
    except SerialForgeError as exc:
        print(f"SerialForge 关闭时协议 parser 仍有 worker 未完成：{exc}", file=sys.stderr)
    try:
        context.components.shutdown(timeout=3.0)
    except SerialForgeError as exc:
        print(f"SerialForge 关闭时组件 worker 仍有任务未完成：{exc}", file=sys.stderr)
    try:
        context.dataset.shutdown(timeout=3.0)
    except SerialForgeError as exc:
        print(f"SerialForge 关闭时 dataset worker 仍有任务未完成：{exc}", file=sys.stderr)
    try:
        context.recorder.shutdown(timeout=3.0)
    except SerialForgeError as exc:
        print(f"SerialForge 关闭时记录器仍有数据未完成：{exc}", file=sys.stderr)
    return exit_code
