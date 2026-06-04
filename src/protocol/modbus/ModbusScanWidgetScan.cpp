/**
 * @file ModbusScanWidgetScan.cpp
 * @brief Modbus从站地址扫描器 — 扫描逻辑实现
 *
 * 从 ModbusScanWidget.cpp 拆分而来，包含:
 *   - setModbusMaster(): 设置ModbusMaster实例并绑定响应/超时/异常信号
 *   - startScan(): 开始扫描指定地址范围
 *   - stopScan(): 停止正在进行的扫描，恢复UI状态
 *   - scanNext(): 发送下一个探测请求并更新进度条
 *   - updateScanButtonState(): 更新扫描按钮文字
 *
 * 构造/UI构建保留在 ModbusScanWidget.cpp。
 * 统计/结果/表格更新方法见 ModbusScanWidgetStats.cpp。
 */

#include "protocol/modbus/ModbusScanWidget.h"

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
    ++m_totalScansCompleted;
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

/** @brief 根据用户选择的功能码发送下一个探测请求并更新进度条 */
void ModbusScanWidget::scanNext() {
    if (m_currentAddr > m_scanTo || !m_scanning) {
        stopScan();
        return;
    }

    m_progressBar->setValue(m_currentAddr - m_scanFrom);
    emit scanProgress(m_currentAddr);
    ++m_totalAddressesProbed;

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
