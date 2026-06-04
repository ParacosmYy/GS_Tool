/**
 * @file RegisterEditor.cpp
 * @brief 寄存器编辑器实现
 */

#include "connection/spi_i2c/RegisterEditor.h"
#include "connection/interface/IConnection.h"
#include "connection/spi_i2c/I2cConnection.h"
#include "connection/spi_i2c/SpiConnection.h"
#include <QHBoxLayout>

/** @brief 构造寄存器编辑器UI，初始化地址/数据输入和操作日志 @param parent 父控件 */
RegisterEditor::RegisterEditor(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("RegisterEditor");
    setupUi();
    setupConnections();
}

/** @brief 设置底层I2C/SPI连接实例 @param connection IConnection实例 */
void RegisterEditor::setConnection(IConnection* connection)
{
    m_connection = connection;
}

/** @brief 设置默认读取长度 @param length 读取字节数 */
void RegisterEditor::setReadLength(int length)
{
    m_readLength = qMax(1, length);
    if (m_lengthSpin) {
        m_lengthSpin->setValue(m_readLength);
    }
}

/** @brief 读取指定地址的寄存器，根据连接类型分派I2C/SPI读操作 @param address 寄存器地址 */
void RegisterEditor::readAddress(int address)
{
    if (!m_connection) {
        appendLog(tr("R [0x%1] → 错误: 未连接").arg(address, 2, 16, QChar('0')), false);
        ++m_totalErrors;
        return;
    }

    int len = m_lengthSpin ? m_lengthSpin->value() : m_readLength;
    QByteArray data;

    /// 根据连接类型分派读操作
    auto* i2c = qobject_cast<I2cConnection*>(m_connection);
    if (i2c) {
        data = i2c->readRegister(m_deviceAddress, address, len);
    } else {
        /// SPI或其他类型: 使用通用write/read方式
        QByteArray cmd;
        cmd.append(static_cast<char>(address & 0xFF));
        m_connection->write(cmd);
    }

    /// 记录日志
    QString addrStr = QString("0x%1").arg(address, 2, 16, QChar('0')).toUpper();
    if (data.isEmpty()) {
        appendLog(tr("R 地址: %1 -> (无数据)").arg(addrStr), false);
    } else {
        appendLog(tr("R 地址: %1 -> 数据: %2")
            .arg(addrStr)
            .arg(formatHex(data)), false);
    }

    ++m_readCount;
    ++m_totalRegisterReads;
    emit registerReadComplete(address, data);
}

/** @brief 向指定地址写入数据，根据连接类型分派I2C/SPI写操作 @param address 寄存器地址 @param data 待写入数据 */
void RegisterEditor::writeAddress(int address, const QByteArray& data)
{
    if (!m_connection) {
        appendLog(tr("W [0x%1] ← 错误: 未连接").arg(address, 2, 16, QChar('0')), true);
        ++m_totalErrors;
        return;
    }

    bool success = false;

    /// 根据连接类型分派写操作
    auto* i2c = qobject_cast<I2cConnection*>(m_connection);
    if (i2c) {
        success = i2c->writeRegister(0x00, address, data);
    } else {
        QByteArray cmd;
        cmd.append(static_cast<char>(address & 0xFF));
        cmd.append(data);
        qint64 written = m_connection->write(cmd);
        success = (written > 0);
    }

    QString addrStr = QString("0x%1").arg(address, 2, 16, QChar('0')).toUpper();
    if (success) {
        appendLog(tr("W 地址: %1 <- 数据: %2 [成功]")
            .arg(addrStr)
            .arg(formatHex(data)), true);
    } else {
        appendLog(tr("W 地址: %1 <- 数据: %2 [失败]")
            .arg(addrStr)
            .arg(formatHex(data)), true);
    }

    emit registerWriteComplete(address, success);
    ++m_writeCount;
    ++m_totalRegisterWrites;
}

/** @brief 读取按钮点击处理，从地址输入框读取并执行读操作 */
void RegisterEditor::onReadClicked()
{
    if (!m_addrSpin) return;
    readAddress(m_addrSpin->value());
}

/** @brief 写入按钮点击处理，从地址和数据输入框读取并执行写操作 */
void RegisterEditor::onWriteClicked()
{
    if (!m_addrSpin || !m_dataEdit) return;
    int addr = m_addrSpin->value();
    QByteArray data = QByteArray::fromHex(m_dataEdit->text().toUtf8());
    writeAddress(addr, data);
}

/** @brief 清空日志按钮点击处理 */
void RegisterEditor::onClearLogClicked()
{
    if (m_log) {
        m_log->clear();
        ++m_totalLogClears;
    }
}

/** @brief 初始化UI布局: 地址/长度输入行、数据输入、读/写/清空按钮、操作日志 */
// 日志格式化/统计getter/resetStatistics见 RegisterEditorStats.cpp
