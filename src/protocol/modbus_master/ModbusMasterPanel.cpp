/**
 * @file ModbusMasterPanel.cpp
 * @brief Modbus Master交互面板实现 -- 左侧配置+右侧响应表格+轮询控制
 *
 * 左侧: 从站地址/功能码/寄存器/数量/值/超时/重试。
 * 右侧: 响应数据表格(寄存器地址+HEX+DEC)+轮询间隔+启停。
 * 统计接口见 ModbusMasterPanelStats.cpp。
 */
#include "protocol/modbus_master/ModbusMasterPanel.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHeaderView>
#include <QSplitter>

// ============================================================================
// 构造
// ============================================================================

/** @brief 构造面板: 创建控件+布局+信号连接 @param poller 轮询调度器 @param parent 父控件 */
ModbusMasterPanel::ModbusMasterPanel(ModbusMasterPoller* poller, QWidget* parent)
    : QWidget(parent)
    , m_poller(poller)
{
    setObjectName(QStringLiteral("ModbusMasterPanel"));
    setupUI();

    /* 连接轮询调度器信号 */
    if (m_poller) {
        connect(m_poller, &ModbusMasterPoller::responseReceived,
                this, &ModbusMasterPanel::onResponseReceived);
        connect(m_poller, &ModbusMasterPoller::requestSent,
                this, &ModbusMasterPanel::onRequestSent);
        connect(m_poller, &ModbusMasterPoller::errorOccurred,
                this, &ModbusMasterPanel::onErrorOccurred);
    }
}

// ============================================================================
// UI构建
// ============================================================================

/** @brief 创建整体布局: 水平分割器(左侧配置+右侧响应) */
void ModbusMasterPanel::setupUI() {
    auto* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setObjectName(QStringLiteral("mainSplitter"));

    /* 左侧: 配置区 */
    auto* configGroup = createConfigGroup();
    splitter->addWidget(configGroup);

    /* 右侧: 响应区 */
    auto* responseGroup = createResponseGroup();
    splitter->addWidget(responseGroup);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    mainLayout->addWidget(splitter);
}

/** @brief 创建左侧配置分组 @return 配置QGroupBox */
QGroupBox* ModbusMasterPanel::createConfigGroup() {
    auto* group = new QGroupBox(tr("请求配置"), this);
    group->setObjectName(QStringLiteral("configGroup"));
    auto* form = new QFormLayout(group);

    /* 从站地址 */
    m_slaveSpin = new QSpinBox(this);
    m_slaveSpin->setObjectName(QStringLiteral("slaveAddrSpin"));
    m_slaveSpin->setRange(1, 247);
    m_slaveSpin->setValue(1);
    form->addRow(tr("从站地址:"), m_slaveSpin);

    /* 功能码 */
    m_funcCombo = new QComboBox(this);
    m_funcCombo->setObjectName(QStringLiteral("funcCodeCombo"));
    m_funcCombo->addItem(tr("FC01 读线圈"),      static_cast<int>(ModbusMasterFunction::ReadCoils));
    m_funcCombo->addItem(tr("FC02 读离散输入"),   static_cast<int>(ModbusMasterFunction::ReadDiscreteInputs));
    m_funcCombo->addItem(tr("FC03 读保持寄存器"), static_cast<int>(ModbusMasterFunction::ReadHoldingRegisters));
    m_funcCombo->addItem(tr("FC04 读输入寄存器"), static_cast<int>(ModbusMasterFunction::ReadInputRegisters));
    m_funcCombo->addItem(tr("FC05 写单个线圈"),   static_cast<int>(ModbusMasterFunction::WriteSingleCoil));
    m_funcCombo->addItem(tr("FC06 写单个寄存器"), static_cast<int>(ModbusMasterFunction::WriteSingleRegister));
    m_funcCombo->addItem(tr("FC16 写多个寄存器"), static_cast<int>(ModbusMasterFunction::WriteMultipleRegisters));
    m_funcCombo->setCurrentIndex(2);
    form->addRow(tr("功能码:"), m_funcCombo);

    /* 起始寄存器 */
    m_startRegSpin = new QSpinBox(this);
    m_startRegSpin->setObjectName(QStringLiteral("startRegSpin"));
    m_startRegSpin->setRange(0, 65535);
    m_startRegSpin->setValue(0);
    m_startRegSpin->setPrefix(QStringLiteral("0x"));
    form->addRow(tr("起始寄存器:"), m_startRegSpin);

    /* 数量 */
    m_countSpin = new QSpinBox(this);
    m_countSpin->setObjectName(QStringLiteral("countSpin"));
    m_countSpin->setRange(1, 125);
    m_countSpin->setValue(10);
    form->addRow(tr("数量:"), m_countSpin);

    /* 写入值 */
    m_valueSpin = new QSpinBox(this);
    m_valueSpin->setObjectName(QStringLiteral("valueSpin"));
    m_valueSpin->setRange(0, 65535);
    m_valueSpin->setDisplayIntegerBase(16);
    form->addRow(tr("写入值(HEX):"), m_valueSpin);

    /* 超时 */
    m_timeoutSpin = new QSpinBox(this);
    m_timeoutSpin->setObjectName(QStringLiteral("timeoutSpin"));
    m_timeoutSpin->setRange(100, 30000);
    m_timeoutSpin->setValue(1000);
    m_timeoutSpin->setSuffix(tr(" ms"));
    m_timeoutSpin->setSingleStep(100);
    form->addRow(tr("超时:"), m_timeoutSpin);

    /* 重试次数 */
    m_retrySpin = new QSpinBox(this);
    m_retrySpin->setObjectName(QStringLiteral("retrySpin"));
    m_retrySpin->setRange(0, 10);
    m_retrySpin->setValue(3);
    form->addRow(tr("最大重试:"), m_retrySpin);

    /* 发送按钮 */
    m_sendBtn = new QPushButton(tr("发送请求"), this);
    m_sendBtn->setObjectName(QStringLiteral("sendBtn"));
    form->addRow(m_sendBtn);
    connect(m_sendBtn, &QPushButton::clicked, this, &ModbusMasterPanel::onSendClicked);

    return group;
}

/** @brief 创建右侧响应分组 @return 响应QGroupBox */
QGroupBox* ModbusMasterPanel::createResponseGroup() {
    auto* group = new QGroupBox(tr("响应数据"), this);
    group->setObjectName(QStringLiteral("responseGroup"));
    auto* layout = new QVBoxLayout(group);

    /* 响应表格: 寄存器地址 | HEX | DEC */
    m_responseTable = new QTableWidget(0, 3, this);
    m_responseTable->setObjectName(QStringLiteral("responseTable"));
    m_responseTable->setHorizontalHeaderLabels(
        {tr("寄存器"), tr("HEX"), tr("DEC")});
    m_responseTable->horizontalHeader()->setStretchLastSection(true);
    m_responseTable->horizontalHeader()->setSectionResizeMode(
        0, QHeaderView::ResizeToContents);
    m_responseTable->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::ResizeToContents);
    m_responseTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_responseTable->setAlternatingRowColors(true);
    layout->addWidget(m_responseTable);

    /* 轮询控制行 */
    auto* pollLayout = new QHBoxLayout();
    pollLayout->addWidget(new QLabel(tr("轮询间隔:"), this));
    m_pollIntervalSpin = new QSpinBox(this);
    m_pollIntervalSpin->setObjectName(QStringLiteral("pollIntervalSpin"));
    m_pollIntervalSpin->setRange(50, 60000);
    m_pollIntervalSpin->setValue(500);
    m_pollIntervalSpin->setSuffix(tr(" ms"));
    m_pollIntervalSpin->setSingleStep(50);
    pollLayout->addWidget(m_pollIntervalSpin);

    m_pollBtn = new QPushButton(tr("开始轮询"), this);
    m_pollBtn->setObjectName(QStringLiteral("pollBtn"));
    m_pollBtn->setCheckable(true);
    pollLayout->addWidget(m_pollBtn);
    pollLayout->addStretch();
    layout->addLayout(pollLayout);

    /* 状态/错误标签 */
    m_statusLabel = new QLabel(tr("就绪"), this);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
    layout->addWidget(m_statusLabel);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setObjectName(QStringLiteral("errorLabel"));
    layout->addWidget(m_errorLabel);

    /* 轮询按钮切换 */
    connect(m_pollBtn, &QPushButton::toggled, this, &ModbusMasterPanel::onPollToggle);

    return group;
}

// ============================================================================
// 槽函数
// ============================================================================

/** @brief 发送按钮点击: 从UI构建请求并提交 */
void ModbusMasterPanel::onSendClicked() {
    if (!m_poller) return;
    ++m_totalSendClicks;

    /* 同步配置到poller */
    m_poller->setTimeout(m_timeoutSpin->value());
    m_poller->setMaxRetries(m_retrySpin->value());

    ModbusMasterRequest req;
    req.slaveAddr = static_cast<quint8>(m_slaveSpin->value());
    req.func = static_cast<ModbusMasterFunction>(
        m_funcCombo->currentData().toInt());
    req.startReg = static_cast<quint16>(m_startRegSpin->value());
    req.count = static_cast<quint16>(m_countSpin->value());

    /* 写操作: 填充values */
    if (req.func == ModbusMasterFunction::WriteSingleCoil ||
        req.func == ModbusMasterFunction::WriteSingleRegister) {
        req.values.append(static_cast<quint16>(m_valueSpin->value()));
    } else if (req.func == ModbusMasterFunction::WriteMultipleRegisters) {
        /* 批量写: 用value作为起始值递增 */
        quint16 base = static_cast<quint16>(m_valueSpin->value());
        for (int i = 0; i < req.count; ++i) {
            req.values.append(base + i);
        }
    }

    if (!m_poller->sendRequest(req)) {
        m_errorLabel->setText(tr("发送失败(可能正在等待响应)"));
    }
}

/** @brief 轮询启停切换 */
void ModbusMasterPanel::onPollToggle() {
    if (!m_poller) return;
    ++m_totalPollToggles;

    if (m_pollBtn->isChecked()) {
        /* 启动轮询: 先同步配置 */
        m_poller->setTimeout(m_timeoutSpin->value());
        m_poller->setMaxRetries(m_retrySpin->value());
        m_poller->setPollInterval(m_pollIntervalSpin->value());
        m_pollBtn->setText(tr("停止轮询"));
        m_statusLabel->setText(tr("轮询中..."));

        /* 添加当前配置为轮询条目 */
        m_poller->clearPollEntries();
        ModbusMasterRequest req;
        req.slaveAddr = static_cast<quint8>(m_slaveSpin->value());
        req.func = static_cast<ModbusMasterFunction>(
            m_funcCombo->currentData().toInt());
        req.startReg = static_cast<quint16>(m_startRegSpin->value());
        req.count = static_cast<quint16>(m_countSpin->value());
        m_poller->addPollEntry(req);
        m_poller->startPolling();
    } else {
        m_poller->stopPolling();
        m_pollBtn->setText(tr("开始轮询"));
        m_statusLabel->setText(tr("已停止"));
    }
}

/** @brief 收到响应: 更新响应表格 */
void ModbusMasterPanel::onResponseReceived(const ModbusMasterResponse& resp) {
    ++m_totalResponseUpdates;
    updateResponseTable(resp);
    updateStatusLabel();

    if (!resp.success) {
        m_errorLabel->setText(resp.error);
    } else {
        m_errorLabel->clear();
    }
}

/** @brief 请求已发送: 更新状态标签 */
void ModbusMasterPanel::onRequestSent(const ModbusMasterRequest& req) {
    Q_UNUSED(req)
    m_statusLabel->setText(tr("等待响应..."));
}

/** @brief 错误通知 */
void ModbusMasterPanel::onErrorOccurred(const QString& msg) {
    m_errorLabel->setText(msg);
}

// ============================================================================
// 辅助方法
// ============================================================================

/** @brief 更新响应数据表格(寄存器地址/HEX/DEC) @param resp 响应结构体 */
void ModbusMasterPanel::updateResponseTable(const ModbusMasterResponse& resp) {
    m_responseTable->setRowCount(resp.values.size());
    for (int i = 0; i < resp.values.size(); ++i) {
        quint16 addr = resp.startReg + i;
        quint16 val = resp.values[i];
        auto* addrItem = new QTableWidgetItem(
            QStringLiteral("0x%1").arg(addr, 4, 16, QLatin1Char('0')).toUpper());
        auto* hexItem = new QTableWidgetItem(
            QStringLiteral("0x%1").arg(val, 4, 16, QLatin1Char('0')).toUpper());
        auto* decItem = new QTableWidgetItem(QString::number(val));
        m_responseTable->setItem(i, 0, addrItem);
        m_responseTable->setItem(i, 1, hexItem);
        m_responseTable->setItem(i, 2, decItem);
    }
}

/** @brief 更新状态标签(请求/响应/超时/错误统计) */
void ModbusMasterPanel::updateStatusLabel() {
    if (!m_poller) return;
    m_statusLabel->setText(tr("REQ:%1 OK:%2 TMO:%3 ERR:%4")
        .arg(m_poller->totalRequests())
        .arg(m_poller->totalResponses())
        .arg(m_poller->totalTimeouts())
        .arg(m_poller->totalErrors()));
}
