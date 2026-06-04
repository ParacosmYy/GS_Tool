/**
 * @file ModbusScanWidgetStats.cpp
 * @brief Modbus从站地址扫描器 — 统计/结果/表格更新方法
 *
 * 从 ModbusScanWidget.cpp 拆分，包含:
 *   - 统计接口 (计数器访问、重置)
 *   - 扫描响应处理 (onResponseReceived/onScanTimeout/onScanError)
 *   - 设备类型推断 (inferDeviceType)
 *   - 结果查询 (foundSlaves)
 *   - 统计标签刷新 (updateStatsLabel)
 */
#include "protocol/modbus/ModbusScanWidget.h"
#include <QTableWidgetItem>

// ============================================================================
// 扫描响应回调
// ============================================================================

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
    auto* fcItem = new QTableWidgetItem(tr("FC%1").arg(fc, 2, 10, QChar('0')));
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

// ============================================================================
// 设备类型推断
// ============================================================================

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

// ============================================================================
// 结果查询
// ============================================================================

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

// ============================================================================
// 统计接口
// ============================================================================

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
