/**
 * @file SerialConfigPanelUISignals.cpp
 * @brief 串口配置面板 — 控制信号与自动重连UI构建方法
 *
 * 从 SerialConfigPanelUI.cpp 拆分而来，包含控制信号区域和自动重连布局:
 *   - createControlSignalsGroup(): DTR/RTS/Break控制信号按钮+输入信号LED
 *   - createAutoReconnectLayout(): 自动重连复选框+间隔微调框
 */

#include "serial/config/SerialConfigPanel.h"
#include "shared/TimerConstants.h"

#include <QHBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>

#include "core/widgets/AnimatedButton.h"

// ---- 控制信号 + 自动重连 ----

/** @brief 创建DTR/RTS控制信号分组(含按钮、工具提示、信号连接和视觉刷新) */
QGroupBox* SerialConfigPanel::createControlSignalsGroup()
{
    auto* signalGroup = new QGroupBox(tr("控制信号"));
    signalGroup->setObjectName("signalGroup");
    auto* signalLayout = new QHBoxLayout(signalGroup);

    m_dtrBtn = new QPushButton(tr("DTR HIGH"));
    m_dtrBtn->setObjectName("dtrBtn");
    m_dtrBtn->setCheckable(true);
    m_dtrBtn->setChecked(true);
    m_dtrBtn->setEnabled(false);
    m_dtrBtn->setToolTip(tr("数据终端就绪信号，点击切换 HIGH/LOW\n"
                            "部分设备需要 DTR 拉低才能复位(如 ESP32)"));

    m_rtsBtn = new QPushButton(tr("RTS HIGH"));
    m_rtsBtn->setObjectName("rtsBtn");
    m_rtsBtn->setCheckable(true);
    m_rtsBtn->setChecked(true);
    m_rtsBtn->setEnabled(false);
    m_rtsBtn->setToolTip(tr("请求发送信号，点击切换 HIGH/LOW\n"
                             "部分设备需要 RTS 拉低进入 bootloader(如 STM32)"));

    m_breakBtn = new QPushButton(tr("BRK"));
    m_breakBtn->setObjectName("breakBtn");
    m_breakBtn->setToolTip(tr("发送Break信号(用于STM32/ESP32进入Bootloader)"));
    m_breakBtn->setEnabled(false);
    connect(m_breakBtn, &QPushButton::clicked, this, [this]() { emit breakRequested(Timers::kBreakDurationMs); });

    signalLayout->addWidget(m_dtrBtn);
    signalLayout->addWidget(m_rtsBtn);
    signalLayout->addWidget(m_breakBtn);

    // ---- 输入信号线状态LED ----
    auto makeLed = [](const char* n) -> QLabel* {
        auto* l = new QLabel(n); l->setObjectName("signalLed");
        l->setAlignment(Qt::AlignCenter); l->setFixedSize(36, 20);
        l->setProperty("active", false); return l;
    };
    m_ctsLed = makeLed("CTS"); m_dsrLed = makeLed("DSR");
    m_dcdLed = makeLed("DCD"); m_riLed = makeLed("RI");
    for (auto* w : {m_ctsLed, m_dsrLed, m_dcdLed, m_riLed}) signalLayout->addWidget(w);
    signalLayout->addStretch();

    connect(m_dtrBtn, &QPushButton::toggled, this, [this](bool checked) {
        m_dtrState = checked;
        m_dtrBtn->setText(checked ? tr("DTR HIGH") : tr("DTR LOW"));
        refreshSignalStyle(m_dtrBtn, m_dtrState);
        emit dtrChanged(checked);
    });
    connect(m_rtsBtn, &QPushButton::toggled, this, [this](bool checked) {
        m_rtsState = checked;
        m_rtsBtn->setText(checked ? tr("RTS HIGH") : tr("RTS LOW"));
        refreshSignalStyle(m_rtsBtn, m_rtsState);
        emit rtsChanged(checked);
    });
    // 初始化视觉状态为默认HIGH
    refreshSignalStyle(m_dtrBtn, m_dtrState);
    refreshSignalStyle(m_rtsBtn, m_rtsState);
    return signalGroup;
}

/** @brief 创建自动重连控件布局(复选框+间隔微调框, 范围500~30000ms, 步进500ms, 默认3000ms) */
QHBoxLayout* SerialConfigPanel::createAutoReconnectLayout()
{
    auto* lay = new QHBoxLayout;
    m_autoReconnectCheck = new QCheckBox(tr("自动重连"));
    m_autoReconnectCheck->setObjectName("autoReconnectCheck");
    m_reconnectIntervalSpin = new QSpinBox;
    m_reconnectIntervalSpin->setObjectName("reconnectIntervalSpin");
    m_reconnectIntervalSpin->setRange(500, 30000);
    m_reconnectIntervalSpin->setSingleStep(500);
    m_reconnectIntervalSpin->setValue(3000);
    m_reconnectIntervalSpin->setSuffix("ms");
    m_reconnectIntervalSpin->setEnabled(false);
    lay->addWidget(m_autoReconnectCheck);
    QLabel* intervalLbl = new QLabel(tr("间隔"));
    intervalLbl->setObjectName("reconnectIntervalLabel");
    lay->addWidget(intervalLbl);
    lay->addWidget(m_reconnectIntervalSpin);
    connect(m_autoReconnectCheck, &QCheckBox::toggled, this, [this](bool on) {
        m_reconnectIntervalSpin->setEnabled(on);
        emit autoReconnectToggled(on, m_reconnectIntervalSpin->value());
    });
    connect(m_reconnectIntervalSpin, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int v) {
        if (m_autoReconnectCheck->isChecked()) emit autoReconnectToggled(true, v);
    });
    return lay;
}
