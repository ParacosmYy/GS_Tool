"""Theme-aware shell selectors for the SerialForge presentation layer.

Only semantic palette roles belong here.  Widget composition and connection
state remain owned by their controllers; this module only renders QSS.
"""

from __future__ import annotations

from .theme_tokens import ThemeSpec


def build_shell_theme_override(theme: ThemeSpec) -> str:
    """Render shell, connection-state, and status-badge selectors."""

    return f"""
QMainWindow, QWidget#appRoot {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {theme.background}, stop:0.58 {theme.surface},
                                stop:1 {theme.background});
    color: {theme.text};
}}

QDialog {{
    background: {theme.background};
    color: {theme.text};
}}

QWidget {{
    color: {theme.text};
}}

QLabel[role="muted"] {{
    color: {theme.text_muted};
}}

QLabel[role="subtle"] {{
    color: {theme.text_subtle};
}}

QLabel#sendShortcutHint {{
    color: {theme.text_subtle};
    background: {theme.history_surface};
    border-color: {theme.history_border};
}}

QLabel[role="section"] {{
    color: {theme.text};
    border-color: {theme.history_border};
    border-left-color: {theme.accent_pink};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {theme.history_surface}, stop:1 {theme.surface});
}}

QLabel[role="section"][scope="station"] {{
    color: {theme.text};
    border-left-color: {theme.success_border};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {theme.info_surface}, stop:1 {theme.surface});
}}

QLabel[role="error"] {{
    color: {theme.error};
    background: {theme.error_surface};
    border-color: {theme.error_border};
}}

QLabel[role="status"] {{
    color: {theme.accent_purple};
    background: {theme.history_surface};
    border-color: {theme.history_border};
}}

QFrame#extensionStationOverview {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {theme.info_surface}, stop:1 {theme.surface});
    border-color: {theme.info_border};
    border-left-color: {theme.accent};
}}

QLabel#extensionStationOverviewState {{
    color: {theme.accent_blue};
    background: {theme.info_surface};
    border-color: {theme.info_border};
}}

QLabel#extensionStationOverviewValue {{
    color: {theme.accent};
}}

QPushButton#extensionCapabilityCard {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {theme.surface}, stop:1 {theme.surface_input});
    border-color: {theme.border};
    border-left-color: {theme.border};
}}

QPushButton#extensionCapabilityCard[state="contract_only"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {theme.info_surface}, stop:1 {theme.surface});
    border-color: {theme.info_border};
    border-left-color: {theme.accent_blue};
}}

QPushButton#extensionCapabilityCard[state="attach_only"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {theme.history_surface}, stop:1 {theme.surface});
    border-color: {theme.history_border};
    border-left-color: {theme.accent_purple};
}}

QPushButton#extensionCapabilityCard:hover {{
    border-color: {theme.accent_blue};
}}

QPushButton#extensionCapabilityCard[state="contract_only"]:hover {{
    border-color: {theme.accent_blue};
    border-left-color: {theme.accent_blue};
}}

QPushButton#extensionCapabilityCard[state="attach_only"]:hover {{
    border-color: {theme.accent_purple};
    border-left-color: {theme.accent_purple};
}}

QPushButton#extensionCapabilityCard[selected="true"] {{
    border-color: {theme.accent_blue};
    border-left-color: {theme.accent};
}}

QPushButton#extensionCapabilityCard:focus {{
    border-color: {theme.accent_pink};
    border-left-color: {theme.accent_pink};
}}

QFrame#extensionCapabilityDetail {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {theme.info_surface}, stop:1 {theme.surface});
    border-color: {theme.info_border};
    border-left-color: {theme.accent};
}}

QFrame#extensionCapabilityDetail[state="attach_only"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {theme.history_surface}, stop:1 {theme.surface});
    border-color: {theme.history_border};
    border-left-color: {theme.accent_purple};
}}

QLabel#extensionCapabilityDetailEyebrow {{
    color: {theme.accent_blue};
}}

QLabel#extensionCapabilityDetailTitle {{
    color: {theme.text};
}}

QLabel#extensionCapabilityDetailReference {{
    color: {theme.text_muted};
}}

QLabel#dataActivity {{
    color: {theme.accent_pink};
    background: {theme.history_surface};
    border-color: {theme.history_border};
}}

QLabel#dataActivity[state="active"] {{
    color: {theme.success};
    background: {theme.success_surface};
    border-color: {theme.success_border};
}}

QLabel#emptyState {{
    color: {theme.text_subtle};
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
}}

QLabel#brandTitle {{
    color: {theme.accent_pink};
}}

QLabel#brandSubtitle {{
    color: {theme.text_muted};
}}

QFrame#appHeader {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {theme.history_surface},
                                stop:0.48 {theme.surface},
                                stop:1 {theme.info_surface});
    border-color: {theme.history_border};
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
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
}}

QFrame#appHeader[density="compact"] QFrame#statusCluster,
QFrame#appHeader[density="compact"] QFrame#motionControls,
QFrame#appHeader[density="compact"] QFrame#themeControls {{
    border-radius: 8px;
}}

QComboBox#themePicker {{
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 {theme.history_surface}, stop:1 {theme.surface_input});
    border-color: {theme.history_border};
    font-weight: 700;
}}

QComboBox#themePicker:hover {{
    border-color: {theme.accent_purple};
}}

QComboBox#themePicker:focus {{
    border-color: {theme.focus};
}}

QComboBox#themePicker:disabled {{
    color: {theme.disabled_text};
    background: {theme.disabled_surface};
    border-color: {theme.disabled_border};
}}

QFrame#statusCluster[state="discovered"] {{
    background: {theme.info_surface};
    border-color: {theme.info_border};
}}

QFrame#statusCluster[state="opening"],
QFrame#statusCluster[state="closing"] {{
    background: {theme.warning_surface};
    border-color: {theme.warning_border};
}}

QFrame#statusCluster[state="open"] {{
    background: {theme.success_surface};
    border-color: {theme.success_border};
}}

QFrame#statusCluster[state="error"] {{
    background: {theme.error_surface};
    border-color: {theme.error_border};
}}

QFrame#statusCluster[state="closed"] {{
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
}}

QFrame#connectionControlBand {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 {theme.history_surface}, stop:1 {theme.surface});
    border-color: {theme.neutral_border};
    border-left-color: {theme.accent_purple};
}}

QFrame#connectionControlBand[state="discovered"] {{
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {theme.info_surface}, stop:1 {theme.surface});
    border-color: {theme.info_border};
    border-left-color: {theme.accent_blue};
}}

QFrame#connectionControlBand[state="opening"],
QFrame#connectionControlBand[state="closing"] {{
    background: {theme.warning_surface};
    border-color: {theme.warning_border};
    border-left-color: {theme.warning};
}}

QFrame#connectionControlBand[state="open"] {{
    background: {theme.success_surface};
    border-color: {theme.success_border};
    border-left-color: {theme.success};
}}

QFrame#connectionControlBand[state="closed"] {{
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
    border-left-color: {theme.accent_purple};
}}

QFrame#connectionControlBand[state="error"] {{
    background: {theme.error_surface};
    border-color: {theme.error_border};
    border-left-color: {theme.error};
}}

QFrame#connectionPresetContext {{
    color: {theme.text_muted};
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
}}

QFrame#connectionPresetContext[source="builtin"] {{
    color: {theme.accent_blue};
    background: {theme.info_surface};
    border-color: {theme.info_border};
}}

QFrame#connectionPresetContext[source="custom"] {{
    color: {theme.accent_purple};
    background: {theme.history_surface};
    border-color: {theme.history_border};
}}

QFrame#connectionPresetContext QLabel#connectionPresetContextTitle {{
    color: {theme.text};
    font-weight: 700;
}}

QFrame#connectionPresetContext QLabel#connectionHint {{
    color: {theme.text_muted};
}}

QFrame[role="stationBand"] {{
    background: {theme.surface};
    border-color: {theme.border};
}}

QFrame#liveObservationBand[role="stationBand"] {{
    border-left-color: {theme.accent};
}}

QFrame#sendControlBand[role="stationBand"] {{
    border-left-color: {theme.accent_pink};
}}

QFrame[role="stationBand"][source="live"] {{
    background: {theme.success_surface};
    border-color: {theme.success_border};
}}

QFrame[role="stationBand"][source="history"] {{
    background: {theme.history_surface};
    border-color: {theme.history_border};
}}

QFrame#liveObservationBand[role="stationBand"][state="active"],
QFrame#sendControlBand[role="stationBand"][state="ready"] {{
    background: {theme.success_surface};
    border-color: {theme.success_border};
    border-left-color: {theme.success};
}}

QFrame#liveObservationBand[role="stationBand"][state="transition"],
QFrame#liveObservationBand[role="stationBand"][state="paused"],
QFrame#sendControlBand[role="stationBand"][state="busy"] {{
    background: {theme.warning_surface};
    border-color: {theme.warning_border};
    border-left-color: {theme.warning};
}}

QFrame#liveObservationBand[role="stationBand"][state="recording"] {{
    background: {theme.history_surface};
    border-color: {theme.history_border};
    border-left-color: {theme.accent_pink};
}}

QFrame#liveObservationBand[role="stationBand"][state="error"] {{
    background: {theme.error_surface};
    border-color: {theme.error_border};
    border-left-color: {theme.error};
}}

QFrame#sendControlBand[role="stationBand"][state="history"] {{
    background: {theme.history_surface};
    border-color: {theme.history_border};
    border-left-color: {theme.accent_purple};
}}

QFrame#sendControlBand[role="stationBand"][state="waiting"] {{
    background: {theme.info_surface};
    border-color: {theme.info_border};
    border-left-color: {theme.accent_blue};
}}

QFrame#sendControlBand[role="stationBand"][state="blocked"] {{
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
    border-left-color: {theme.accent_purple};
}}

QLabel#stateValue {{
    color: {theme.success};
    background: {theme.success_surface};
    border-color: {theme.success_border};
}}

QLabel#stateValue[state="discovered"] {{
    color: {theme.accent_blue};
    background: {theme.info_surface};
    border-color: {theme.info_border};
}}

QLabel#stateValue[state="opening"],
QLabel#stateValue[state="closing"] {{
    color: {theme.warning};
    background: {theme.warning_surface};
    border-color: {theme.warning_border};
}}

QLabel#stateValue[state="closed"] {{
    color: {theme.text_muted};
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
}}

QLabel#stateValue[state="error"] {{
    color: {theme.error};
    background: {theme.error_surface};
    border-color: {theme.error_border};
}}

QLabel#sendState {{
    color: {theme.text_muted};
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
}}

QLabel#sendState[state="ready"] {{
    color: {theme.success};
    background: {theme.success_surface};
    border-color: {theme.success_border};
}}

QLabel#sendState[state="busy"] {{
    color: {theme.warning};
    background: {theme.warning_surface};
    border-color: {theme.warning_border};
}}

QLabel#sendState[state="history"] {{
    color: {theme.accent_purple};
    background: {theme.history_surface};
    border-color: {theme.history_border};
}}

QLabel#sendState[state="waiting"] {{
    color: {theme.accent_blue};
    background: {theme.info_surface};
    border-color: {theme.info_border};
}}

QLabel#sendContext {{
    color: {theme.text_muted};
    background: {theme.info_surface};
    border-color: {theme.info_border};
}}

QLabel#sendContext[state="empty"] {{
    color: {theme.text_subtle};
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
}}

QLabel#sendContext[state="ready"] {{
    color: {theme.text};
    background: {theme.info_surface};
    border-color: {theme.info_border};
}}

QLabel#sendContext[state="invalid"] {{
    color: {theme.error};
    background: {theme.error_surface};
    border-color: {theme.error_border};
}}

QLabel#commandBatchStatus {{
    color: {theme.text_muted};
    background: {theme.surface_input};
    border-color: {theme.neutral_border};
}}

QLabel#commandBatchStatus[state="empty"] {{
    color: {theme.text_subtle};
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
}}

QLabel#commandBatchStatus[state="ready"],
QLabel#commandBatchStatus[state="completed"] {{
    color: {theme.success};
    background: {theme.success_surface};
    border-color: {theme.success_border};
}}

QLabel#commandBatchStatus[state="running"],
QLabel#commandBatchStatus[state="stopped"] {{
    color: {theme.warning};
    background: {theme.warning_surface};
    border-color: {theme.warning_border};
}}

QLabel#commandBatchStatus[state="failed"] {{
    color: {theme.error};
    background: {theme.error_surface};
    border-color: {theme.error_border};
}}

QFrame#componentEmptyState {{
    color: {theme.text};
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 {theme.surface}, stop:1 {theme.surface_input});
    border-color: {theme.neutral_border};
    border-left-color: {theme.accent_blue};
}}

QFrame#componentEmptyState[state="blocked"] {{
    color: {theme.text_muted};
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
    border-left-color: {theme.warning};
}}

QLabel#componentEmptyEyebrow {{
    color: {theme.text_subtle};
}}

QLabel#componentEmptyTitle {{
    color: {theme.text};
}}

QLabel#componentEmptyHint {{
    color: {theme.text_muted};
}}

QLabel#connectionPresetLabel {{
    color: {theme.accent_blue};
}}

QComboBox#transportCombo {{
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 {theme.info_surface}, stop:1 {theme.surface_input});
    border-color: {theme.info_border};
    font-weight: 700;
}}

QComboBox#transportCombo:hover {{
    border-color: {theme.accent_blue};
}}

QComboBox#transportCombo:focus {{
    border-color: {theme.focus};
}}

QComboBox#transportCombo:disabled {{
    color: {theme.disabled_text};
    background: {theme.disabled_surface};
    border-color: {theme.disabled_border};
}}

QComboBox#connectionPresetCombo {{
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 {theme.info_surface}, stop:1 {theme.surface_input});
    border-color: {theme.info_border};
}}

QComboBox#connectionPresetCombo:hover {{
    border-color: {theme.accent_blue};
}}

QComboBox#connectionPresetCombo:focus {{
    border-color: {theme.focus};
}}

QComboBox#connectionPresetCombo[customSelected="true"] {{
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 {theme.history_surface}, stop:1 {theme.surface_input});
    border-color: {theme.history_border};
}}

QComboBox#connectionPresetCombo[customSelected="true"]:hover {{
    border-color: {theme.accent_purple};
}}

QComboBox#connectionPresetCombo[customSelected="true"]:focus {{
    border-color: {theme.focus};
}}

QComboBox#connectionPresetCombo[customSelected="true"]:disabled {{
    color: {theme.disabled_text};
    background: {theme.disabled_surface};
    border-color: {theme.disabled_border};
}}

QLabel#connectionContext {{
    color: {theme.text_muted};
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
}}

QLabel#sourceBadge {{
    color: {theme.text};
    background: {theme.history_surface};
    border-color: {theme.history_border};
}}

QLabel#sourceBadge[source="live"] {{
    color: {theme.success};
    background: {theme.success_surface};
    border-color: {theme.success_border};
}}

QLabel#sourceBadge[source="history"] {{
    color: {theme.accent_purple};
    background: {theme.history_surface};
    border-color: {theme.history_border};
}}

QLabel#pipelineSummary {{
    color: {theme.text_muted};
    background: {theme.surface_input};
    border-color: {theme.neutral_border};
}}

QLabel#protocolConfigContext {{
    color: {theme.text_muted};
    background: {theme.surface_input};
    border-color: {theme.neutral_border};
}}

QLabel#protocolConfigContext[state="waiting"] {{
    color: {theme.accent_blue};
    background: {theme.info_surface};
    border-color: {theme.info_border};
}}

QLabel#protocolConfigContext[state="active"] {{
    color: {theme.success};
    background: {theme.success_surface};
    border-color: {theme.success_border};
}}

QLabel#protocolConfigContext[state="draft"] {{
    color: {theme.text};
    background: {theme.history_surface};
    border-color: {theme.history_border};
}}

QLabel#protocolConfigContext[state="history"] {{
    color: {theme.accent_purple};
    background: {theme.history_surface};
    border-color: {theme.history_border};
}}

QLabel#protocolConfigContext[state="blocked"],
QLabel#protocolConfigContext[state="idle"] {{
    color: {theme.text_subtle};
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
}}

QLabel#pipelineSummary[state="active"] {{
    color: {theme.success};
    background: {theme.success_surface};
    border-color: {theme.success_border};
    border-left-color: {theme.success};
}}

QLabel#pipelineSummary[state="transition"] {{
    color: {theme.warning};
    background: {theme.warning_surface};
    border-color: {theme.warning_border};
    border-left-color: {theme.warning};
}}

QLabel#pipelineSummary[state="blocked"] {{
    color: {theme.text_subtle};
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
    border-left-color: {theme.warning};
}}

QLabel#pipelineSummary[state="draft"] {{
    color: {theme.text};
    background: {theme.history_surface};
    border-color: {theme.history_border};
    border-left-color: {theme.accent_pink};
}}

QLabel#pipelineSummary[source="history"],
QLabel#pipelineSummary[state="history"] {{
    color: {theme.accent_purple};
    background: {theme.history_surface};
    border-color: {theme.history_border};
    border-left-color: {theme.accent_purple};
}}

QLabel#protocolStatus,
QLabel#componentStatus,
QLabel#datasetStatus,
QLabel#datasetCurveStatus,
QLabel#replayStatus {{
    color: {theme.text_muted};
    background: {theme.surface_input};
    border-color: {theme.neutral_border};
}}

QLabel#protocolStatus[state="active"],
QLabel#componentStatus[state="active"],
QLabel#datasetStatus[state="active"],
QLabel#datasetCurveStatus[state="active"],
QLabel#replayStatus[state="active"] {{
    color: {theme.success};
    background: {theme.success_surface};
    border-color: {theme.success_border};
}}

QLabel#protocolStatus[state="waiting"],
QLabel#componentStatus[state="waiting"],
QLabel#datasetStatus[state="waiting"],
QLabel#datasetCurveStatus[state="waiting"],
QLabel#replayStatus[state="waiting"] {{
    color: {theme.accent_blue};
    background: {theme.info_surface};
    border-color: {theme.info_border};
}}

QLabel#protocolStatus[state="empty"],
QLabel#componentStatus[state="empty"],
QLabel#datasetStatus[state="empty"],
QLabel#datasetCurveStatus[state="empty"],
QLabel#replayStatus[state="idle"],
QLabel#protocolStatus[state="blocked"],
QLabel#componentStatus[state="blocked"],
QLabel#datasetStatus[state="blocked"],
QLabel#datasetCurveStatus[state="blocked"],
QLabel#replayStatus[state="blocked"] {{
    color: {theme.text_subtle};
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
}}

QLabel#protocolStatus[state="error"],
QLabel#componentStatus[state="error"],
QLabel#datasetStatus[state="error"],
QLabel#datasetCurveStatus[state="error"],
QLabel#replayStatus[state="error"] {{
    color: {theme.error};
    background: {theme.error_surface};
    border-color: {theme.error_border};
}}

QLabel#protocolStatus[state="draft"] {{
    color: {theme.text};
    background: {theme.history_surface};
    border-color: {theme.history_border};
}}

QLabel#replayStatus[state="paused"] {{
    color: {theme.warning};
    background: {theme.warning_surface};
    border-color: {theme.warning_border};
}}

QLabel#protocolStatus[source="history"],
QLabel#componentStatus[source="history"],
QLabel#datasetStatus[source="history"],
QLabel#datasetCurveStatus[source="history"],
QLabel#replayStatus[source="history"] {{
    color: {theme.accent_purple};
    background: {theme.history_surface};
    border-color: {theme.history_border};
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
    color: {theme.accent_purple};
    background: {theme.history_surface};
    border-color: {theme.history_border};
    border-left-color: {theme.accent_purple};
}}

QLabel#pipelineSummary[source="history"][state="blocked"],
QLabel#protocolStatus[source="history"][state="blocked"],
QLabel#componentStatus[source="history"][state="blocked"],
QLabel#datasetStatus[source="history"][state="blocked"],
QLabel#datasetCurveStatus[source="history"][state="blocked"],
QLabel#replayStatus[source="history"][state="blocked"] {{
    color: {theme.text_subtle};
    background: {theme.neutral_surface};
    border-color: {theme.neutral_border};
    border-left-color: {theme.warning};
}}

QLabel#pipelineSummary[source="history"][state="error"],
QLabel#protocolStatus[source="history"][state="error"],
QLabel#componentStatus[source="history"][state="error"],
QLabel#datasetStatus[source="history"][state="error"],
QLabel#datasetCurveStatus[source="history"][state="error"],
QLabel#replayStatus[source="history"][state="error"] {{
    color: {theme.error};
    background: {theme.error_surface};
    border-color: {theme.error_border};
    border-left-color: {theme.error};
}}
"""


__all__ = ["build_shell_theme_override"]
