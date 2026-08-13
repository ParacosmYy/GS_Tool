"""Theme tokens for the read-only embedded extension station surfaces."""

from __future__ import annotations

from .theme_tokens import (
    ACCENT,
    ACCENT_BLUE,
    ACCENT_PINK,
    ACCENT_PURPLE,
    BORDER,
    BORDER_STRONG,
    HISTORY_BORDER,
    HISTORY_SURFACE,
    INFO_BORDER,
    INFO_SURFACE,
    SURFACE,
    SURFACE_INPUT,
    TEXT,
    TEXT_MUTED,
)

EXTENSION_STYLESHEET = f"""
QLabel#extensionCapabilityTitle {{
    color: {TEXT};
    background: transparent;
    border: none;
    padding: 0;
    font-weight: 700;
}}

QLabel#extensionCapabilityState {{
    color: {ACCENT_BLUE};
    background: {INFO_SURFACE};
    border: 1px solid {INFO_BORDER};
    border-radius: 6px;
    padding: 2px 7px;
    font-size: 9pt;
    font-weight: 600;
}}

QLabel#extensionCapabilityState[state="attach_only"] {{
    color: {ACCENT_PURPLE};
    background: {HISTORY_SURFACE};
    border-color: {HISTORY_BORDER};
}}

QFrame#extensionStationOverview {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {INFO_SURFACE}, stop:1 {SURFACE});
    border: 1px solid {INFO_BORDER};
    border-left: 3px solid {ACCENT};
    border-radius: 12px;
}}

QLabel#extensionStationOverviewState {{
    color: {ACCENT_BLUE};
    background: {INFO_SURFACE};
    border: 1px solid {INFO_BORDER};
    border-radius: 6px;
    padding: 2px 7px;
    font-size: 9pt;
    font-weight: 600;
}}

QLabel#extensionStationOverviewValue {{
    color: {ACCENT};
    font-size: 12pt;
    font-weight: 700;
}}

QPushButton#extensionCapabilityCard {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {SURFACE}, stop:1 {SURFACE_INPUT});
    border: 1px solid {BORDER};
    border-left: 3px solid {BORDER_STRONG};
    border-radius: 10px;
    padding: 10px;
    min-height: 122px;
    text-align: left;
}}

QPushButton#extensionCapabilityCard[state="contract_only"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {INFO_SURFACE}, stop:1 {SURFACE});
    border-color: {INFO_BORDER};
    border-left-color: {ACCENT_BLUE};
}}

QPushButton#extensionCapabilityCard[state="attach_only"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {HISTORY_SURFACE}, stop:1 {SURFACE});
    border-color: {HISTORY_BORDER};
    border-left-color: {ACCENT_PURPLE};
}}

QPushButton#extensionCapabilityCard:hover {{
    border-color: {ACCENT_BLUE};
}}

QPushButton#extensionCapabilityCard[state="contract_only"]:hover {{
    border-color: {ACCENT_BLUE};
    border-left-color: {ACCENT_BLUE};
}}

QPushButton#extensionCapabilityCard[state="attach_only"]:hover {{
    border-color: {ACCENT_PURPLE};
    border-left-color: {ACCENT_PURPLE};
}}

QPushButton#extensionCapabilityCard[selected="true"] {{
    border-color: {ACCENT_BLUE};
    border-left-color: {ACCENT};
}}

QPushButton#extensionCapabilityCard:focus {{
    border-color: {ACCENT_PINK};
    border-left-color: {ACCENT_PINK};
}}

QFrame#extensionCapabilityDetail {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {INFO_SURFACE}, stop:1 {SURFACE});
    border: 1px solid {INFO_BORDER};
    border-left: 3px solid {ACCENT};
    border-radius: 10px;
}}

QFrame#extensionCapabilityDetail[state="attach_only"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {HISTORY_SURFACE}, stop:1 {SURFACE});
    border-color: {HISTORY_BORDER};
    border-left-color: {ACCENT_PURPLE};
}}

QLabel#extensionCapabilityDetailEyebrow {{
    color: {ACCENT_BLUE};
    background: transparent;
    border: none;
    padding: 0;
    font-size: 9pt;
    font-weight: 700;
}}

QLabel#extensionCapabilityDetailTitle {{
    color: {TEXT};
    background: transparent;
    border: none;
    padding: 0;
    font-size: 12pt;
    font-weight: 700;
}}

QLabel#extensionCapabilityDetailReference {{
    color: {TEXT_MUTED};
    background: transparent;
    border: none;
    padding: 0;
    font-size: 9pt;
}}
"""

__all__ = ["EXTENSION_STYLESHEET"]
