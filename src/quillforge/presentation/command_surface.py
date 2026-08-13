"""Presentation projection for the desktop menu and command rail."""

from __future__ import annotations

from collections.abc import Callable, Sequence

from PyQt6.QtCore import QSize, Qt
from PyQt6.QtGui import QAction, QKeySequence, QPalette
from PyQt6.QtWidgets import QLabel, QMainWindow, QMenu, QSizePolicy, QToolBar, QWidget

from ..application.commands import SUPPORTED_MENU_IDS, CommandRegistry
from ..domain.models import Locale
from .i18n import command_title, tr
from .icon_contract import IconKey
from .icons import themed_icon
from .toolbar_contract import ToolbarActionRole, ToolbarActionSpec

__all__ = ["CommandSurface", "ToolbarActionRole", "ToolbarActionSpec"]


class CommandSurface:
    """Own the menu/toolbar projection without owning command behavior."""

    _MENU_KEYS: tuple[tuple[str, str], ...] = tuple(
        (menu_id, f"menu.{menu_id}") for menu_id in SUPPORTED_MENU_IDS
    )

    def __init__(
        self,
        parent: QMainWindow,
        commands: CommandRegistry,
        locale_provider: Callable[[], Locale],
    ) -> None:
        self._parent = parent
        self._commands = commands
        self._locale_provider = locale_provider
        self._menus: dict[str, QMenu] = {}
        self._menu_actions: list[QAction] = []
        self._toolbar_actions: dict[str, QAction] = {}
        self._toolbar_icon_keys: dict[str, IconKey] = {}
        self._toolbar_brand: QLabel | None = None
        self._toolbar_context: QLabel | None = None
        self._toolbar: QToolBar | None = None

    def _locale(self) -> Locale:
        """Resolve the current locale through the composition-owned provider."""
        return self._locale_provider()

    def create_menus(self) -> None:
        """Create the stable top-level menus and project registered commands."""
        self._menus = {
            menu_id: self._parent.menuBar().addMenu(tr(text_key, self._locale()))
            for menu_id, text_key in self._MENU_KEYS
        }
        self.refresh()

    def create_toolbar(self, actions: Sequence[ToolbarActionSpec]) -> None:
        """Create the compact command rail from explicit callback specs."""
        toolbar = QToolBar(tr("toolbar.command_rail", self._locale()), self._parent)
        toolbar.setObjectName("commandBar")
        toolbar.setMovable(False)
        toolbar.setFloatable(False)
        toolbar.setIconSize(QSize(18, 18))
        toolbar.setToolButtonStyle(Qt.ToolButtonStyle.ToolButtonTextBesideIcon)
        self._parent.addToolBar(Qt.ToolBarArea.TopToolBarArea, toolbar)
        self._toolbar = toolbar
        self._toolbar_actions.clear()
        self._toolbar_icon_keys.clear()

        brand = QLabel(toolbar)
        brand.setObjectName("toolbarBrand")
        brand.setAccessibleName(tr("app.title", self._locale()))
        brand.setToolTip(tr("app.title", self._locale()))
        brand.setText(f"✦ {tr('app.title', self._locale())}")
        toolbar.addWidget(brand)
        toolbar.addSeparator()
        self._toolbar_brand = brand

        for spec in actions:
            if spec.separator_before:
                toolbar.addSeparator()
            title = tr(spec.text_key, self._locale())
            action = QAction(title, self._parent)
            action.setToolTip(title)
            action.triggered.connect(lambda _checked=False, callback=spec.callback: callback())
            toolbar.addAction(action)
            button = toolbar.widgetForAction(action)
            if button is not None:
                button.setProperty("commandRole", spec.role)
            self._toolbar_actions[spec.text_key] = action
            if spec.icon_key is not None:
                self._toolbar_icon_keys[spec.text_key] = spec.icon_key

        spacer = QWidget(toolbar)
        spacer.setSizePolicy(QSizePolicy.Policy.Expanding, QSizePolicy.Policy.Preferred)
        toolbar.addWidget(spacer)
        context = QLabel(tr("toolbar.context", self._locale()), toolbar)
        context.setObjectName("toolbarContext")
        toolbar.addWidget(context)
        self._toolbar_context = context
        self._refresh_toolbar_icons()

    def refresh(self) -> None:
        """Rebuild menu actions after command registration or plugin changes."""
        for action in self._menu_actions:
            action.setEnabled(False)
            action.setShortcut(QKeySequence())
            action.deleteLater()
        self._menu_actions.clear()
        for menu in self._menus.values():
            menu.clear()

        locale = self._locale()
        for command in self._commands.all():
            menu = self._menus.get(command.menu_id)
            if menu is None:
                continue
            action = QAction(command_title(command.command_id, command.title, locale), self._parent)
            if command.shortcut is not None:
                action.setShortcut(QKeySequence(command.shortcut))
            action.triggered.connect(lambda _checked=False, command=command: command.execute())
            menu.addAction(action)
            self._menu_actions.append(action)

    def retranslate(self, locale: Locale | None = None) -> None:
        """Refresh menu titles, command labels, and command-rail text."""
        current_locale = locale or self._locale()
        self.refresh()
        for menu_id, text_key in self._MENU_KEYS:
            menu = self._menus.get(menu_id)
            if menu is not None:
                menu.setTitle(tr(text_key, current_locale))
        for text_key, action in self._toolbar_actions.items():
            title = tr(text_key, current_locale)
            action.setText(title)
            action.setToolTip(title)
        if self._toolbar is not None:
            self._toolbar.setWindowTitle(tr("toolbar.command_rail", current_locale))
        if self._toolbar_brand is not None:
            title = tr("app.title", current_locale)
            self._toolbar_brand.setAccessibleName(title)
            self._toolbar_brand.setToolTip(title)
            self._toolbar_brand.setText(f"✦ {title}")
        if self._toolbar_context is not None:
            self._toolbar_context.setText(tr("toolbar.context", current_locale))
        self._refresh_toolbar_icons()

    def refresh_command_menus(self) -> None:
        """Compatibility name for the MainWindow/application lifecycle seam."""
        self.refresh()

    def _refresh_toolbar_icons(self) -> None:
        """Retint authored icons from the palette after a theme application."""
        palette = self._parent.palette()
        foreground = palette.color(QPalette.ColorRole.ButtonText).name()
        accent = palette.color(QPalette.ColorRole.Link).name()
        disabled_foreground = palette.color(
            QPalette.ColorGroup.Disabled,
            QPalette.ColorRole.ButtonText,
        ).name()
        for text_key, icon_key in self._toolbar_icon_keys.items():
            action = self._toolbar_actions.get(text_key)
            if action is not None:
                action.setIcon(
                    themed_icon(
                        icon_key,
                        foreground=foreground,
                        accent=accent,
                        disabled_foreground=disabled_foreground,
                        disabled_accent=disabled_foreground,
                    )
                )
