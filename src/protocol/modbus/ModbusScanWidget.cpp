/**
 * @file ModbusScanWidget.cpp
 * @brief Modbus从站地址扫描器实现
 *
 * 逐地址发送探测请求，收集在线从站列表并更新表格。
 * 提供地址范围输入、探测功能码选择、进度条、结果表格和统计面板。
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
    rangeLayout->addWidget(new QLabel(tr("起始地址:"), this));

    m_fromSpin = new QSpinBox(this);
    m_fromSpin->setObjectName("fromSpin");
    m_fromSpin->setRange(1, 247);
    m_fromSpin->setValue(1);
    rangeLayout->addWidget(m_fromSpin);

    rangeLayout->addWidget(new QLabel(tr("结束地址:"), this));

    m_toSpin = new QSpinBox(this);
    m_toSpin->setObjectName("toSpin");
    m_toSpin->setRange(1, 247);
    m_toSpin->setValue(247);
    rangeLayout->addWidget(m_toSpin);

    rangeLayout->addSpacing(16);
    rangeLayout->addWidget(new QLabel(tr("探测方式:"), this));

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

/** @brief 设置ModbusMaster实例(绑定响应/超时/异常信号) @param master Modbus主站指针 */
void ModbusScanWidget::setModbusMaster(ModbusMaster* master) {
    /* 使用static_cast消除信号名重载歧义(QObject::timeout/error) */
    using TimeoutSig  = void (ModbusMaster::*)(int, int);
    using ErrorSig    = void (ModbusMaster::*)(ModbusError);

    if (m_master) {
        disconnect(m_master, &ModbusMaster::responseReceived,
                   this, &ModbusScanWidget::onResponseReceived);
        disconnect(m_master, static_cast<TimeoutSig>(&ModbusMaster::timeout),
                   this, &ModbusScanWidget::onScanTimeout);
        disconnect(m_master, static_cast<ErrorSig>(&ModbusMaster::error),
                   this, &ModbusScanWidget::onScanError);
    }
    m_master = master;
    if (m_master) {
        connect(m_master, &ModbusMaster::responseReceived,
                this, &ModbusScanWidget::onResponseReceived);
        connect(m_master, static_cast<TimeoutSig>(&ModbusMaster::timeout),
                this, &ModbusScanWidget::onScanTimeout);
        connect(m_master, static_cast<ErrorSig>(&ModbusMaster::error),
                this, &ModbusScanWidget::onScanError);
    }
}

/** @brief 开始扫描指定地址范围 @param from 起始地址 @param to 结束地址 */
void ModbusScanWidget::startScan(int from, int to) {
    m_scanFrom    = from;
    m_scanTo      = to;
    m_currentAddr = from;
    m_scanning    = true;
    ++m_totalScansInitiated;

    // 清空上次结果
    m_resultTable->setRowCount(0);

    // 配置进度条
    int totalSteps = to - from + 1;
    m_progressBar->setRange(0, totalSteps);
    m_progressBar->setValue(0);
    m_progressBar->setFormat(tr("扫描中... %p%"));

    updateScanButtonState(true);

    // 禁用范围输入
    m_fromSpin->setEnabled(false);
    m_toSpin->setEnabled(false);
    m_probeFunc->setEnabled(false);

    scanNext();
}

/** @brief 停止正在进行的扫描，恢复UI状态并发射scanCompleted信号 */
void ModbusScanWidget::stopScan() {
    m_scanning = false;
    updateScanButtonState(false);

    // 恢复范围输入
    m_fromSpin->setEnabled(true);
    m_toSpin->setEnabled(true);
    m_probeFunc->setEnabled(true);

    int foundCount = m_resultTable->rowCount();
    m_progressBar->setFormat(
        tr("扫描完成 — 发现 %1 个设备").arg(foundCount));

    updateStatsLabel();
    emit scanCompleted();
}

/** @brief 获取已发现的从站地址列表 @return 从站地址列表 */
QList<int> ModbusScanWidget::foundSlaves() const {
    QList<int> slaves;
    for (int row = 0; row < m_resultTable->rowCount(); ++row) {
        QTableWidgetItem* item = m_resultTable->item(row, 0);
        if (item) {
            slaves.append(item->data(Qt::UserRole).toInt());
        }
    }
    return slaves;
}

/** @brief ModbusMaster响应到达回调，解析设备信息添加到表格并继续扫描 @param frame 接收到的Modbus帧 */
void ModbusScanWidget::onResponseReceived(const ModbusFrame& frame) {
    if (!m_scanning) { return; }

    int addr = static_cast<int>(frame.slaveAddress);
    ++m_totalSuccessfulProbes;
    ++m_totalSlavesFound;

    // 推断设备类型
    QString deviceType = inferDeviceType(frame);
    int fc = static_cast<int>(frame.function);

    // 从响应数据中提取设备ID（取前2字节作为设备标识）
    QString deviceId = tr("未知");
    if (!frame.data.isEmpty() && frame.data.size() >= 2) {
        quint16 id = (static_cast<quint8>(frame.data[0]) << 8) |
                      static_cast<quint8>(frame.data[1]);
        deviceId = QString("0x%1").arg(id, 4, 16, QChar('0')).toUpper();
    } else if (!frame.data.isEmpty()) {
        quint8 id = static_cast<quint8>(frame.data[0]);
        deviceId = QString("0x%1").arg(id, 2, 16, QChar('0')).toUpper();
    }

    // 插入表格行
    int row = m_resultTable->rowCount();
    m_resultTable->insertRow(row);

    // 从站地址列
    auto* addrItem = new QTableWidgetItem(QString::number(addr));
    addrItem->setData(Qt::UserRole, addr);
    addrItem->setTextAlignment(Qt::AlignCenter);
    m_resultTable->setItem(row, 0, addrItem);

    // 设备ID列
    auto* idItem = new QTableWidgetItem(deviceId);
    idItem->setTextAlignment(Qt::AlignCenter);
    m_resultTable->setItem(row, 1, idItem);

    // 设备类型列
    auto* typeItem = new QTableWidgetItem(deviceType);
    m_resultTable->setItem(row, 2, typeItem);

    // 响应功能码列
    auto* fcItem = new QTableWidgetItem(QString("FC%1").arg(fc, 2, 10, QChar('0')));
    fcItem->setTextAlignment(Qt::AlignCenter);
    m_resultTable->setItem(row, 3, fcItem);

    QString desc = tr("从站 %1 — %2").arg(addr).arg(deviceType);
    emit slaveFound(addr, desc);

    // 更新进度
    int progress = m_currentAddr - m_scanFrom;
    m_progressBar->setValue(progress);
    updateStatsLabel();

    scanNext();
}

/** @brief 超时回调：递增超时计数，跳过当前地址继续扫描 @param slave 超时从站地址 @param function 超时功能码 */
void ModbusScanWidget::onScanTimeout(int slave, int function) {
    Q_UNUSED(slave)
    Q_UNUSED(function)
    if (!m_scanning) { return; }

    ++m_totalTimeouts;

    int progress = m_currentAddr - m_scanFrom;
    m_progressBar->setValue(progress);
    updateStatsLabel();

    scanNext();
}

/** @brief 异常响应回调：递增错误计数，继续扫描 @param errorCode Modbus异常码 */
void ModbusScanWidget::onScanError(ModbusError errorCode) {
    Q_UNUSED(errorCode)
    if (!m_scanning) { return; }

    ++m_totalErrors;

    int progress = m_currentAddr - m_scanFrom;
    m_progressBar->setValue(progress);
    updateStatsLabel();

    scanNext();
}

/** @brief 根据用户选择的功能码发送下一个探测请求并更新进度条 */
void ModbusScanWidget::scanNext() {
    if (m_currentAddr > m_scanTo || !m_scanning) {
        stopScan();
        return;
    }

    m_progressBar->setValue(m_currentAddr - m_scanFrom);
    emit scanProgress(m_currentAddr);

    // 根据用户选择发送对应功能码探测
    if (m_master) {
        int fc = m_probeFunc->currentData().toInt();
        switch (static_cast<ModbusFunction>(fc)) {
        case ModbusFunction::ReadHoldingRegisters:
            m_master->readHoldingRegisters(m_currentAddr, 0, 1);
            break;
        case ModbusFunction::ReadInputRegisters:
            m_master->readInputRegisters(m_currentAddr, 0, 1);
            break;
        case ModbusFunction::ReadCoils:
            m_master->readCoils(m_currentAddr, 0, 1);
            break;
        default:
            m_master->readHoldingRegisters(m_currentAddr, 0, 1);
            break;
        }
    }
    m_currentAddr++;
}

/** @brief 更新扫描按钮文字(扫描中="停止扫描"/空闲="开始扫描") @param scanning true=正在扫描 */
void ModbusScanWidget::updateScanButtonState(bool scanning) {
    if (scanning) {
        m_scanBtn->setText(tr("停止扫描"));
    } else {
        m_scanBtn->setText(tr("开始扫描"));
    }
    m_scanBtn->setEnabled(true);
}

/** @brief 根据响应帧内容推断设备类型描述 @param frame Modbus响应帧 @return 设备类型字符串 */
QString ModbusScanWidget::inferDeviceType(const ModbusFrame& frame) {
    // 根据功能码和返回数据推断设备类型
    quint8 fc = static_cast<quint8>(frame.function);

    switch (static_cast<ModbusFunction>(fc)) {
    case ModbusFunction::ReadCoils:
    case ModbusFunction::ReadDiscreteInputs:
        return tr("离散IO设备");
    case ModbusFunction::ReadHoldingRegisters:
        return tr("保持寄存器设备");
    case ModbusFunction::ReadInputRegisters:
        return tr("模拟量输入设备");
    case ModbusFunction::WriteSingleCoil:
    case ModbusFunction::WriteMultipleCoils:
        return tr("线圈输出设备");
    case ModbusFunction::WriteSingleRegister:
    case ModbusFunction::WriteMultipleRegisters:
        return tr("寄存器写入设备");
    default:
        return tr("未知设备");
    }
}

/** @brief 更新统计信息标签文字 */
void ModbusScanWidget::updateStatsLabel() {
    m_statsLabel->setText(
        tr("扫描: %1次 | 发现: %2 | 成功: %3 | 超时: %4 | 异常: %5")
            .arg(m_totalScansInitiated)
            .arg(m_totalSlavesFound)
            .arg(m_totalSuccessfulProbes)
            .arg(m_totalTimeouts)
            .arg(m_totalErrors));
}

// ============================================================================
// 统计接口
// ============================================================================

/** @brief 获取累计发起扫描次数 */
quint64 ModbusScanWidget::totalScansInitiated() const {
    return m_totalScansInitiated;
}

/** @brief 获取累计发现的从站总数 */
quint64 ModbusScanWidget::totalSlavesFound() const {
    return m_totalSlavesFound;
}

/** @brief 获取累计成功响应次数 */
quint64 ModbusScanWidget::totalSuccessfulProbes() const {
    return m_totalSuccessfulProbes;
}

/** @brief 获取累计超时次数 */
quint64 ModbusScanWidget::totalTimeouts() const {
    return m_totalTimeouts;
}

/** @brief 获取累计异常响应次数 */
quint64 ModbusScanWidget::totalErrors() const {
    return m_totalErrors;
}

/** @brief 重置所有统计计数器 */
void ModbusScanWidget::resetStatistics() {
    m_totalScansInitiated  = 0;
    m_totalSlavesFound     = 0;
    m_totalSuccessfulProbes = 0;
    m_totalTimeouts        = 0;
    m_totalErrors          = 0;
    updateStatsLabel();
}
