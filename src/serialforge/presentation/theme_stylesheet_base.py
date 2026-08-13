"""Stable base stylesheet for the resource-free SerialForge shell."""

from __future__ import annotations

from .theme_tokens import (
    ACCENT,
    ACCENT_BLUE,
    ACCENT_PINK,
    ACCENT_PURPLE,
    BACKGROUND,
    BORDER,
    BORDER_STRONG,
    DISABLED_BORDER,
    DISABLED_SURFACE,
    DISABLED_TEXT,
    ERROR,
    ERROR_BORDER,
    ERROR_SURFACE,
    FOCUS,
    HISTORY_BORDER,
    HISTORY_SURFACE,
    INFO_BORDER,
    INFO_SURFACE,
    NEUTRAL_BORDER,
    NEUTRAL_SURFACE,
    SUCCESS,
    SUCCESS_BORDER,
    SUCCESS_SURFACE,
    SURFACE,
    SURFACE_INPUT,
    TEXT,
    TEXT_MUTED,
    TEXT_SUBTLE,
    WARNING,
    WARNING_BORDER,
    WARNING_SURFACE,
)

BASE_STYLESHEET = f"""
QMainWindow, QWidget#appRoot {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {BACKGROUND}, stop:0.58 {SURFACE},
                                stop:1 {BACKGROUND});
    color: {TEXT};
}}

QDialog {{
    background: {BACKGROUND};
    color: {TEXT};
}}

QWidget {{
    color: {TEXT};
    font-family: "Microsoft YaHei UI", "Microsoft YaHei", "Segoe UI", "Noto Sans SC", sans-serif;
    font-size: 10pt;
}}

QLabel {{
    background: transparent;
}}

QLabel[role="muted"] {{
    color: {TEXT_MUTED};
}}

QLabel[role="subtle"] {{
    color: {TEXT_SUBTLE};
    font-size: 9pt;
}}

QLabel#sendShortcutHint {{
    color: {TEXT_SUBTLE};
    background: {HISTORY_SURFACE};
    border: 1px solid {BORDER};
    border-radius: 6px;
    padding: 2px 6px;
    font-size: 9pt;
}}

QLabel[role="section"] {{
    color: {TEXT};
    font-weight: 700;
    padding: 4px 10px;
    border: 1px solid {HISTORY_BORDER};
    border-left: 3px solid {ACCENT_PINK};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {HISTORY_SURFACE}, stop:1 {SURFACE});
    border-radius: 8px;
}}

QLabel[role="section"][scope="station"] {{
    color: {TEXT};
    border-left-color: {ACCENT};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {INFO_SURFACE}, stop:1 {SURFACE});
}}

QLabel[role="error"] {{
    color: {ERROR};
    background: {ERROR_SURFACE};
    border: 1px solid {ERROR_BORDER};
    border-radius: 7px;
    padding: 4px 8px;
}}

QLabel[role="status"] {{
    color: {ACCENT_PURPLE};
    background: {HISTORY_SURFACE};
    border: 1px solid {BORDER};
    border-radius: 6px;
    padding: 3px 7px;
}}

QLabel#uartTimingSummary {{
    color: {ACCENT_BLUE};
    background: {INFO_SURFACE};
    border: 1px solid {INFO_BORDER};
    border-left: 3px solid {ACCENT};
    border-radius: 7px;
    padding: 3px 8px;
    font-size: 9pt;
    font-weight: 600;
}}

QLabel#dataActivity {{
    color: {ACCENT_PINK};
    background: {HISTORY_SURFACE};
    border-color: {HISTORY_BORDER};
}}

QLabel#dataActivity[state="active"] {{
    color: {SUCCESS};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {SUCCESS_SURFACE}, stop:1 {INFO_SURFACE});
    border-color: {SUCCESS_BORDER};
    font-weight: 700;
}}

QLabel#emptyState {{
    color: {TEXT_SUBTLE};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {SURFACE_INPUT}, stop:1 {SURFACE});
    border: 1px dashed {BORDER_STRONG};
    border-radius: 9px;
    padding: 7px 10px;
}}

QLabel#brandTitle {{
    color: {ACCENT_PINK};
    font-size: 17pt;
    font-weight: 700;
    letter-spacing: 2px;
}}

QLabel#brandSubtitle {{
    color: {TEXT_MUTED};
    font-size: 9pt;
}}

QFrame#appHeader {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {HISTORY_SURFACE}, stop:0.48 {SURFACE},
                                stop:1 {INFO_SURFACE});
    border: 1px solid {HISTORY_BORDER};
    border-radius: 16px;
}}

QFrame#appHeader[density="compact"] {{
    border-radius: 13px;
}}

QFrame#appHeader[density="compact"] QLabel#brandSubtitle {{
    font-size: 8pt;
}}

QFrame#statusCluster,
QFrame#motionControls,
QFrame#themeControls {{
    background: {NEUTRAL_SURFACE};
    border: 1px solid {NEUTRAL_BORDER};
    border-radius: 10px;
}}

QFrame#appHeader[density="compact"] QFrame#statusCluster,
QFrame#appHeader[density="compact"] QFrame#motionControls,
QFrame#appHeader[density="compact"] QFrame#themeControls {{
    border-radius: 8px;
}}

QComboBox#themePicker {{
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 {HISTORY_SURFACE}, stop:1 {SURFACE_INPUT});
    border-color: {HISTORY_BORDER};
    font-weight: 700;
}}

QComboBox#themePicker:hover {{
    border-color: {ACCENT_PURPLE};
}}

QComboBox#themePicker:focus {{
    border-color: {FOCUS};
}}

QComboBox#themePicker:disabled {{
    color: {DISABLED_TEXT};
    background: {DISABLED_SURFACE};
    border-color: {DISABLED_BORDER};
}}

QFrame#statusCluster[state="discovered"] {{
    background: {INFO_SURFACE};
    border-color: {INFO_BORDER};
}}

QFrame#statusCluster[state="opening"],
QFrame#statusCluster[state="closing"] {{
    background: {WARNING_SURFACE};
    border-color: {WARNING_BORDER};
}}

QFrame#statusCluster[state="open"] {{
    background: {SUCCESS_SURFACE};
    border-color: {SUCCESS_BORDER};
}}

QFrame#statusCluster[state="error"] {{
    background: {ERROR_SURFACE};
    border-color: {ERROR_BORDER};
}}

QFrame#statusCluster[state="closed"] {{
    background: {NEUTRAL_SURFACE};
    border-color: {NEUTRAL_BORDER};
}}

QFrame#connectionControlBand {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {SURFACE}, stop:1 {SURFACE});
    border: 1px solid {NEUTRAL_BORDER};
    border-left: 3px solid {HISTORY_BORDER};
    border-radius: 10px;
}}

QFrame#connectionControlBand[state="discovered"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {INFO_SURFACE}, stop:1 {SURFACE});
    border-color: {INFO_BORDER};
    border-left-color: {ACCENT_BLUE};
}}

QFrame#connectionControlBand[state="opening"],
QFrame#connectionControlBand[state="closing"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {WARNING_SURFACE}, stop:1 {SURFACE});
    border-color: {WARNING_BORDER};
    border-left-color: {WARNING};
}}

QFrame#connectionControlBand[state="open"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {SUCCESS_SURFACE}, stop:1 {SURFACE});
    border-color: {SUCCESS_BORDER};
    border-left-color: {ACCENT};
}}

QFrame#connectionControlBand[state="closed"] {{
    background: {NEUTRAL_SURFACE};
    border-color: {NEUTRAL_BORDER};
    border-left-color: {HISTORY_BORDER};
}}

QFrame#connectionControlBand[state="error"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {ERROR_SURFACE}, stop:1 {SURFACE});
    border-color: {ERROR_BORDER};
    border-left-color: {ERROR};
}}

QFrame#connectionPresetContext {{
    color: {TEXT_MUTED};
    background: {NEUTRAL_SURFACE};
    border: 1px solid {NEUTRAL_BORDER};
    border-radius: 8px;
}}

QFrame#connectionPresetContext[source="builtin"] {{
    color: {ACCENT_BLUE};
    background: {INFO_SURFACE};
    border-color: {INFO_BORDER};
}}

QFrame#connectionPresetContext[source="custom"] {{
    color: {ACCENT_PURPLE};
    background: {HISTORY_SURFACE};
    border-color: {HISTORY_BORDER};
}}

QFrame#connectionPresetContext QLabel#connectionPresetContextTitle {{
    color: {TEXT};
    font-weight: 700;
}}

QFrame#connectionPresetContext QLabel#connectionHint {{
    color: {TEXT_MUTED};
}}

QFrame[role="stationBand"] {{
    background: {SURFACE};
    border: 1px solid {BORDER};
    border-radius: 12px;
}}

QFrame#liveObservationBand[role="stationBand"] {{
    border-left: 3px solid {ACCENT};
}}

QFrame#sendControlBand[role="stationBand"] {{
    border-left: 3px solid {ACCENT_PINK};
}}

QFrame[role="stationBand"][source="live"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {SUCCESS_SURFACE}, stop:1 {SURFACE});
    border-color: {SUCCESS_BORDER};
}}

QFrame#liveObservationBand[role="stationBand"][source="live"] {{
    border-left-color: {ACCENT};
}}

QFrame#sendControlBand[role="stationBand"][source="live"] {{
    border-left-color: {ACCENT_PINK};
}}

QFrame[role="stationBand"][source="history"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {HISTORY_SURFACE}, stop:1 {SURFACE});
    border-color: {HISTORY_BORDER};
}}

QFrame#liveObservationBand[role="stationBand"][source="history"] {{
    border-left-color: {ACCENT_PURPLE};
}}

QFrame#liveObservationBand[role="stationBand"][state="active"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {SUCCESS_SURFACE}, stop:1 {INFO_SURFACE});
    border-color: {SUCCESS_BORDER};
    border-left-color: {ACCENT};
}}

QFrame#liveObservationBand[role="stationBand"][state="transition"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {WARNING_SURFACE}, stop:1 {SURFACE});
    border-color: {WARNING_BORDER};
    border-left-color: {WARNING};
}}

QFrame#liveObservationBand[role="stationBand"][state="paused"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {WARNING_SURFACE}, stop:1 {SURFACE});
    border-color: {WARNING_BORDER};
    border-left-color: {WARNING};
}}

QFrame#liveObservationBand[role="stationBand"][state="recording"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {ERROR_SURFACE}, stop:1 {SURFACE});
    border-color: {ERROR_BORDER};
    border-left-color: {ACCENT_PINK};
}}

QFrame#liveObservationBand[role="stationBand"][state="error"] {{
    background: {ERROR_SURFACE};
    border-color: {ERROR_BORDER};
    border-left-color: {ERROR};
}}

QFrame#sendControlBand[role="stationBand"][source="history"] {{
    border-left-color: {ACCENT_BLUE};
}}

QFrame#sendControlBand[role="stationBand"][state="ready"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {SUCCESS_SURFACE}, stop:1 {SURFACE});
    border-color: {SUCCESS_BORDER};
    border-left-color: {ACCENT};
}}

QFrame#sendControlBand[role="stationBand"][state="busy"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {WARNING_SURFACE}, stop:1 {SURFACE});
    border-color: {WARNING_BORDER};
    border-left-color: {WARNING};
}}

QFrame#sendControlBand[role="stationBand"][state="history"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {HISTORY_SURFACE}, stop:1 {SURFACE});
    border-color: {HISTORY_BORDER};
    border-left-color: {ACCENT_PURPLE};
}}

QFrame#sendControlBand[role="stationBand"][state="waiting"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {INFO_SURFACE}, stop:1 {SURFACE});
    border-color: {INFO_BORDER};
    border-left-color: {ACCENT_BLUE};
}}

QFrame#sendControlBand[role="stationBand"][state="blocked"] {{
    background: {NEUTRAL_SURFACE};
    border-color: {NEUTRAL_BORDER};
    border-left-color: {HISTORY_BORDER};
}}

QLabel#stateValue {{
    color: {ACCENT};
    font-weight: 700;
    padding: 3px 9px;
    background: {SUCCESS_SURFACE};
    border: 1px solid {SUCCESS_BORDER};
    border-radius: 8px;
}}

QLabel#stateValue[state="discovered"] {{
    color: {ACCENT_BLUE};
    background: {INFO_SURFACE};
    border-color: {INFO_BORDER};
}}

QLabel#stateValue[state="opening"],
QLabel#stateValue[state="closing"] {{
    color: {WARNING};
    background: {WARNING_SURFACE};
    border-color: {WARNING_BORDER};
}}

QLabel#stateValue[state="closed"] {{
    color: {TEXT_MUTED};
    background: {NEUTRAL_SURFACE};
    border-color: {NEUTRAL_BORDER};
}}

QLabel#stateValue[state="error"] {{
    color: {ERROR};
    background: {ERROR_SURFACE};
    border-color: {ERROR_BORDER};
}}

QLabel#sendState {{
    color: {TEXT_MUTED};
    background: {NEUTRAL_SURFACE};
    border: 1px solid {NEUTRAL_BORDER};
    border-radius: 8px;
    padding: 3px 8px;
    font-size: 9pt;
    font-weight: 700;
}}

QLabel#sendState[state="ready"] {{
    color: {SUCCESS};
    background: {SUCCESS_SURFACE};
    border-color: {SUCCESS_BORDER};
}}

QLabel#sendState[state="busy"] {{
    color: {WARNING};
    background: {WARNING_SURFACE};
    border-color: {WARNING_BORDER};
}}

QLabel#sendState[state="history"] {{
    color: {TEXT};
    background: {HISTORY_SURFACE};
    border-color: {HISTORY_BORDER};
}}

QLabel#sendState[state="waiting"] {{
    color: {ACCENT_BLUE};
    background: {INFO_SURFACE};
    border-color: {INFO_BORDER};
}}

QLabel#sendContext {{
    color: {TEXT_MUTED};
    background: {INFO_SURFACE};
    border: 1px solid {INFO_BORDER};
    border-radius: 8px;
    padding: 3px 7px;
    font-size: 9pt;
    font-weight: 600;
}}

QLabel#sendContext[state="empty"] {{
    color: {TEXT_SUBTLE};
    background: {SURFACE};
    border-color: {BORDER};
}}

QLabel#sendContext[state="ready"] {{
    color: {TEXT};
    background: {INFO_SURFACE};
    border-color: {INFO_BORDER};
}}

QLabel#sendContext[state="invalid"] {{
    color: {ERROR};
    background: {ERROR_SURFACE};
    border-color: {ERROR_BORDER};
}}

QLabel#commandBatchStatus {{
    color: {TEXT_MUTED};
    background: {NEUTRAL_SURFACE};
    border: 1px solid {NEUTRAL_BORDER};
    border-radius: 8px;
    padding: 6px 10px 6px 28px;
    font-weight: 600;
}}

QLabel#commandBatchStatus[state="empty"] {{
    color: {TEXT_SUBTLE};
    background: {NEUTRAL_SURFACE};
    border-color: {NEUTRAL_BORDER};
}}

QLabel#commandBatchStatus[state="ready"],
QLabel#commandBatchStatus[state="completed"] {{
    color: {SUCCESS};
    background: {SUCCESS_SURFACE};
    border-color: {SUCCESS_BORDER};
}}

QLabel#commandBatchStatus[state="running"],
QLabel#commandBatchStatus[state="stopped"] {{
    color: {WARNING};
    background: {WARNING_SURFACE};
    border-color: {WARNING_BORDER};
}}

QLabel#commandBatchStatus[state="failed"] {{
    color: {ERROR};
    background: {ERROR_SURFACE};
    border-color: {ERROR_BORDER};
}}

QFrame#componentEmptyState {{
    color: {TEXT};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {SURFACE}, stop:1 {SURFACE});
    border: 1px solid {BORDER};
    border-left: 3px solid {ACCENT_BLUE};
    border-radius: 10px;
}}

QFrame#componentEmptyState[state="blocked"] {{
    color: {TEXT_MUTED};
    background: {NEUTRAL_SURFACE};
    border-color: {NEUTRAL_BORDER};
    border-left-color: {WARNING};
}}

QLabel#componentEmptyEyebrow {{
    color: {TEXT_SUBTLE};
    font-size: 8pt;
    font-weight: 700;
    letter-spacing: 1px;
}}

QLabel#componentEmptyTitle {{
    color: {TEXT};
    font-size: 11pt;
    font-weight: 700;
}}

QLabel#componentEmptyHint {{
    color: {TEXT_MUTED};
    font-size: 9pt;
}}

QLabel#connectionPresetLabel {{
    color: {ACCENT_BLUE};
}}

QComboBox#transportCombo {{
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 {INFO_SURFACE}, stop:1 {SURFACE_INPUT});
    border-color: {INFO_BORDER};
    font-weight: 700;
}}

QComboBox#transportCombo:hover {{
    border-color: {ACCENT_BLUE};
}}

QComboBox#transportCombo:focus {{
    border-color: {FOCUS};
}}

QComboBox#transportCombo:disabled {{
    color: {DISABLED_TEXT};
    background: {DISABLED_SURFACE};
    border-color: {DISABLED_BORDER};
}}

QComboBox#connectionPresetCombo {{
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 {INFO_SURFACE}, stop:1 {SURFACE_INPUT});
    border-color: {INFO_BORDER};
}}

QComboBox#connectionPresetCombo:hover {{
    border-color: {ACCENT_BLUE};
}}

QComboBox#connectionPresetCombo:focus {{
    border-color: {FOCUS};
}}

QComboBox#connectionPresetCombo[customSelected="true"] {{
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 {HISTORY_SURFACE}, stop:1 {SURFACE});
    border-color: {HISTORY_BORDER};
}}

QComboBox#connectionPresetCombo[customSelected="true"]:hover {{
    border-color: {ACCENT_PURPLE};
}}

QComboBox#connectionPresetCombo[customSelected="true"]:focus {{
    border-color: {FOCUS};
}}

QComboBox#connectionPresetCombo[customSelected="true"]:disabled {{
    color: {DISABLED_TEXT};
    background: {DISABLED_SURFACE};
    border-color: {DISABLED_BORDER};
}}

QLabel#connectionContext {{
    color: {TEXT_MUTED};
    font-size: 9pt;
    background: {NEUTRAL_SURFACE};
    border: 1px solid {INFO_BORDER};
    border-radius: 8px;
    padding: 4px 8px;
}}

QLabel#sourceBadge {{
    color: {TEXT};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {HISTORY_SURFACE}, stop:1 {INFO_SURFACE});
    border: 1px solid {HISTORY_BORDER};
    border-radius: 8px;
    padding: 4px 9px;
    font-size: 9pt;
    font-weight: 700;
}}

QLabel#sourceBadge[source="live"] {{
    color: {SUCCESS};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {SUCCESS_SURFACE}, stop:1 {INFO_SURFACE});
    border-color: {SUCCESS_BORDER};
}}

QLabel#sourceBadge[source="history"] {{
    color: {TEXT};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {HISTORY_SURFACE}, stop:1 {INFO_SURFACE});
    border-color: {ACCENT_PURPLE};
}}

QLabel#pipelineSummary {{
    color: {TEXT_MUTED};
    background: {SURFACE_INPUT};
    border: 1px solid {BORDER};
    border-left: 3px solid {ACCENT_BLUE};
    border-radius: 8px;
    padding: 6px 9px;
}}

QLabel#protocolConfigContext {{
    color: {TEXT_MUTED};
    background: {SURFACE};
    border: 1px solid {BORDER};
    border-radius: 8px;
    padding: 3px 8px;
    font-size: 9pt;
    font-weight: 600;
}}

QLabel#protocolConfigContext[state="waiting"] {{
    color: {ACCENT_BLUE};
    background: {INFO_SURFACE};
    border-color: {INFO_BORDER};
}}

QLabel#protocolConfigContext[state="active"] {{
    color: {SUCCESS};
    background: {SUCCESS_SURFACE};
    border-color: {SUCCESS_BORDER};
}}

QLabel#protocolConfigContext[state="draft"] {{
    color: {TEXT};
    background: {HISTORY_SURFACE};
    border-color: {HISTORY_BORDER};
}}

QLabel#protocolConfigContext[state="history"] {{
    color: {TEXT};
    background: {HISTORY_SURFACE};
    border-color: {HISTORY_BORDER};
}}

QLabel#protocolConfigContext[state="blocked"],
QLabel#protocolConfigContext[state="idle"] {{
    color: {TEXT_SUBTLE};
    background: {NEUTRAL_SURFACE};
    border-color: {NEUTRAL_BORDER};
}}

QLabel#pipelineSummary[source="live"] {{
    border-left-color: {ACCENT_BLUE};
}}

QLabel#pipelineSummary[source="history"],
QLabel#pipelineSummary[state="history"] {{
    color: {TEXT};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {HISTORY_SURFACE}, stop:1 {SURFACE});
    border-color: {HISTORY_BORDER};
    border-left-color: {ACCENT_PURPLE};
}}

QLabel#pipelineSummary[state="active"] {{
    color: {SUCCESS};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {SUCCESS_SURFACE}, stop:1 {SURFACE});
    border-color: {SUCCESS_BORDER};
    border-left-color: {ACCENT};
}}

QLabel#pipelineSummary[state="transition"] {{
    color: {WARNING};
    background: {WARNING_SURFACE};
    border-color: {WARNING_BORDER};
    border-left-color: {WARNING};
}}

QLabel#pipelineSummary[state="blocked"] {{
    color: {TEXT_SUBTLE};
    background: {NEUTRAL_SURFACE};
    border-color: {NEUTRAL_BORDER};
    border-left-color: {HISTORY_BORDER};
}}

QLabel#pipelineSummary[state="draft"] {{
    color: {TEXT};
    background: {HISTORY_SURFACE};
    border-color: {HISTORY_BORDER};
    border-left-color: {ACCENT_PINK};
}}

QLabel#protocolStatus,
QLabel#componentStatus,
QLabel#datasetStatus,
QLabel#datasetCurveStatus,
QLabel#replayStatus {{
    color: {TEXT_MUTED};
    background: {SURFACE_INPUT};
    border: 1px solid {BORDER};
    border-radius: 8px;
    padding: 3px 8px 3px 22px;
    font-size: 9pt;
    font-weight: 600;
}}

QLabel#protocolStatus[source="history"],
QLabel#componentStatus[source="history"],
QLabel#datasetStatus[source="history"],
QLabel#datasetCurveStatus[source="history"],
QLabel#replayStatus[source="history"] {{
    color: {TEXT};
    background: {HISTORY_SURFACE};
    border-color: {HISTORY_BORDER};
}}

QLabel#protocolStatus[state="active"],
QLabel#componentStatus[state="active"],
QLabel#datasetStatus[state="active"],
QLabel#datasetCurveStatus[state="active"],
QLabel#replayStatus[state="active"] {{
    color: {SUCCESS};
    background: {SUCCESS_SURFACE};
    border-color: {SUCCESS_BORDER};
}}

QLabel#protocolStatus[state="waiting"],
QLabel#componentStatus[state="waiting"],
QLabel#datasetStatus[state="waiting"],
QLabel#datasetCurveStatus[state="waiting"],
QLabel#replayStatus[state="waiting"] {{
    color: {ACCENT_BLUE};
    background: {INFO_SURFACE};
    border-color: {INFO_BORDER};
}}

QLabel#protocolStatus[state="empty"],
QLabel#componentStatus[state="empty"],
QLabel#datasetStatus[state="empty"],
QLabel#datasetCurveStatus[state="empty"],
QLabel#replayStatus[state="idle"] {{
    color: {TEXT_SUBTLE};
    background: {NEUTRAL_SURFACE};
    border-color: {NEUTRAL_BORDER};
}}

QLabel#protocolStatus[state="error"],
QLabel#componentStatus[state="error"],
QLabel#datasetStatus[state="error"],
QLabel#datasetCurveStatus[state="error"],
QLabel#replayStatus[state="error"] {{
    color: {ERROR};
    background: {ERROR_SURFACE};
    border-color: {ERROR_BORDER};
}}

QLabel#protocolStatus[state="blocked"],
QLabel#componentStatus[state="blocked"],
QLabel#datasetStatus[state="blocked"],
QLabel#datasetCurveStatus[state="blocked"],
QLabel#replayStatus[state="blocked"] {{
    color: {TEXT_SUBTLE};
    background: {NEUTRAL_SURFACE};
    border-color: {NEUTRAL_BORDER};
}}

QLabel#protocolStatus[state="draft"] {{
    color: {TEXT};
    background: {HISTORY_SURFACE};
    border-color: {HISTORY_BORDER};
}}

QLabel#replayStatus[state="paused"] {{
    color: {WARNING};
    background: {WARNING_SURFACE};
    border-color: {WARNING_BORDER};
}}

QLabel#replayStatus[state="history"] {{
    color: {TEXT};
    background: {HISTORY_SURFACE};
    border-color: {HISTORY_BORDER};
}}

QLabel#pipelineSummary[source="history"][state="active"],
QLabel#pipelineSummary[source="history"][state="waiting"],
QLabel#pipelineSummary[source="history"][state="empty"],
QLabel#pipelineSummary[source="history"][state="draft"],
QLabel#pipelineSummary[source="history"][state="history"],
QLabel#protocolStatus[source="history"][state="active"],
QLabel#protocolStatus[source="history"][state="waiting"],
QLabel#protocolStatus[source="history"][state="empty"],
QLabel#protocolStatus[source="history"][state="draft"],
QLabel#protocolStatus[source="history"][state="history"],
QLabel#componentStatus[source="history"][state="active"],
QLabel#componentStatus[source="history"][state="waiting"],
QLabel#componentStatus[source="history"][state="empty"],
QLabel#componentStatus[source="history"][state="draft"],
QLabel#componentStatus[source="history"][state="history"],
QLabel#datasetStatus[source="history"][state="active"],
QLabel#datasetStatus[source="history"][state="waiting"],
QLabel#datasetStatus[source="history"][state="empty"],
QLabel#datasetStatus[source="history"][state="draft"],
QLabel#datasetStatus[source="history"][state="history"],
QLabel#datasetCurveStatus[source="history"][state="active"],
QLabel#datasetCurveStatus[source="history"][state="waiting"],
QLabel#datasetCurveStatus[source="history"][state="empty"],
QLabel#datasetCurveStatus[source="history"][state="draft"],
QLabel#datasetCurveStatus[source="history"][state="history"],
QLabel#replayStatus[source="history"][state="active"],
QLabel#replayStatus[source="history"][state="waiting"],
QLabel#replayStatus[source="history"][state="empty"],
QLabel#replayStatus[source="history"][state="idle"],
QLabel#replayStatus[source="history"][state="history"] {{
    color: {TEXT};
    background: {HISTORY_SURFACE};
    border-color: {HISTORY_BORDER};
    border-left-color: {ACCENT_PURPLE};
}}

QLabel#pipelineSummary[source="history"][state="blocked"],
QLabel#protocolStatus[source="history"][state="blocked"],
QLabel#componentStatus[source="history"][state="blocked"],
QLabel#datasetStatus[source="history"][state="blocked"],
QLabel#datasetCurveStatus[source="history"][state="blocked"],
QLabel#replayStatus[source="history"][state="blocked"] {{
    color: {TEXT_SUBTLE};
    background: {NEUTRAL_SURFACE};
    border-color: {NEUTRAL_BORDER};
    border-left-color: {WARNING};
}}

QLabel#pipelineSummary[source="history"][state="error"],
QLabel#protocolStatus[source="history"][state="error"],
QLabel#componentStatus[source="history"][state="error"],
QLabel#datasetStatus[source="history"][state="error"],
QLabel#datasetCurveStatus[source="history"][state="error"],
QLabel#replayStatus[source="history"][state="error"] {{
    color: {ERROR};
    background: {ERROR_SURFACE};
    border-color: {ERROR_BORDER};
    border-left-color: {ERROR};
}}
"""

__all__ = ["BASE_STYLESHEET"]
