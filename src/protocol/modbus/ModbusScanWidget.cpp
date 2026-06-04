/**
 * @file ModbusScanWidget.cpp
 * @brief Modbus从站地址扫描器实现 — 构造/UI/扫描逻辑
 *
 * 逐地址发送探测请求，收集在线从站列表并更新表格。
 * 提供地址范围输入、探测功能码选择、进度条、结果表格和统计面板。
 *
 * 统计/结果/表格更新方法已拆分至 ModbusScanWidgetStats.cpp。
 */
#include "protocol/modbus/ModbusScanWidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QGroupBox>

/** @brief 构造Modbus从站扫描器(地址范围+功能码选择+进度条+结果表格+统计) @param parent 父控件 */
ModbusScanWidget::ModbusScanWidget(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("ModbusScanWidget");

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    // ---- 地址范围行 ----
    auto* rangeLayout = new QHBoxLayout();
    auto* fromLbl = new QLabel(tr("起始地址:"), this);
    fromLbl->setObjectName("scanFromLabel");
    rangeLayout->addWidget(fromLbl);

    m_fromSpin = new QSpinBox(this);
    m_fromSpin->setObjectName("fromSpin");
    m_fromSpin->setRange(1, 247);
    m_fromSpin->setValue(1);
    rangeLayout->addWidget(m_fromSpin);

    auto* toLbl = new QLabel(tr("结束地址:"), this);
    toLbl->setObjectName("scanToLabel");
    rangeLayout->addWidget(toLbl);

    m_toSpin = new QSpinBox(this);
    m_toSpin->setObjectName("toSpin");
    m_toSpin->setRange(1, 247);
    m_toSpin->setValue(247);
    rangeLayout->addWidget(m_toSpin);

    rangeLayout->addSpacing(16);
    auto* probeLbl = new QLabel(tr("探测方式:"), this);
    probeLbl->setObjectName("scanProbeLabel");
    rangeLayout->addWidget(probeLbl);

    m_probeFunc = new QComboBox(this);
    m_probeFunc->setObjectName("probeFuncCombo");
    m_probeFunc->addItem(tr("读保持寄存器 (FC03)"), static_cast<int>(ModbusFunction::ReadHoldingRegisters));
    m_probeFunc->addItem(tr("读输入寄存器 (FC04)"), static_cast<int>(ModbusFunction::ReadInputRegisters));
    m_probeFunc->addItem(tr("读线圈 (FC01)"), static_cast<int>(ModbusFunction::ReadCoils));
    rangeLayout->addWidget(m_probeFunc);

    rangeLayout->addStretch();
    mainLayout->addLayout(rangeLayout);

    // ---- 进度条 + 扫描按钮 ----
    auto* progressLayout = new QHBoxLayout();

    m_progressBar = new QProgressBar(this);
    m_progressBar->setObjectName("scanProgressBar");
    m_progressBar->setRange(0, 247);
    m_progressBar->setTextVisible(true);
    m_progressBar->setFormat(tr("就绪"));
    progressLayout->addWidget(m_progressBar, 1);

    m_scanBtn = new QPushButton(tr("开始扫描"), this);
    m_scanBtn->setObjectName("scanButton");
    m_scanBtn->setFixedWidth(100);
    progressLayout->addWidget(m_scanBtn);

    mainLayout->addLayout(progressLayout);

    // ---- 结果表格 ----
    m_resultTable = new QTableWidget(this);
    m_resultTable->setObjectName("scanResultTable");
    m_resultTable->setColumnCount(4);
    m_resultTable->setHorizontalHeaderLabels({
        tr("从站地址"), tr("设备ID"), tr("设备类型"), tr("响应功能码")
    });
    m_resultTable->horizontalHeader()->setStretchLastSection(true);
    m_resultTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    m_resultTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_resultTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_resultTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_resultTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_resultTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_resultTable->setAlternatingRowColors(true);
    mainLayout->addWidget(m_resultTable, 1);

    // ---- 统计标签 ----
    m_statsLabel = new QLabel(this);
    m_statsLabel->setObjectName("scanStatsLabel");
    m_statsLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    updateStatsLabel();
    mainLayout->addWidget(m_statsLabel);

    // ---- 信号连接 ----
    connect(m_scanBtn, &QPushButton::clicked, this, [this]() {
        if (m_scanning) {
            stopScan();
        } else {
            startScan(m_fromSpin->value(), m_toSpin->value());
        }
    });
}

// 扫描逻辑(setModbusMaster/startScan/stopScan/scanNext/updateScanButtonState)见 ModbusScanWidgetScan.cpp
