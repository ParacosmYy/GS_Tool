/**
 * @file BleGattBrowserSlots.cpp
 * @brief GATT浏览器 - 操作槽函数与统计接口实现
 *
 * 从 BleGattBrowser.cpp 拆分而来，包含读写操作、通知订阅/模拟、
 * 写入格式切换和所有统计getter/resetStats方法。
 */

#include "connection/ble/BleGattBrowser.h"
#include "connection/ble/BleConnection.h"
#include <QRandomGenerator>

/** @brief 读取按钮: 模拟读取选中特征的值 */
void BleGattBrowser::onReadClicked()
{
    if (m_selectedUuid.isEmpty()) { m_valueDisplay->append(tr("[错误] 请先选择一个特征")); return; }
    if (!m_connection || m_connection->state() != ConnectionState::Connected) {
        m_valueDisplay->append(tr("[错误] BLE未连接")); return;
    }

    const int dataSize = 4 + QRandomGenerator::global()->bounded(13);
    QByteArray mockData(dataSize, 0);
    for (int i = 0; i < mockData.size(); ++i)
        mockData[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));

    m_valueDisplay->setText(formatHexDump(mockData));
    m_valueDisplay->append(tr("\n[读取完成] %1 (%2 bytes)").arg(m_selectedUuid, QString::number(mockData.size())));
    ++m_totalCharacteristicReads;
    m_totalBytesRead += static_cast<quint64>(mockData.size());
    emit characteristicReadComplete(m_selectedUuid, mockData);
}

/** @brief 写入按钮: 将输入值写入选中特征 */
void BleGattBrowser::onWriteClicked()
{
    const QString inputStr = m_writeInput->text().trimmed();
    if (inputStr.isEmpty()) { m_valueDisplay->append(tr("[错误] 请输入写入值")); return; }
    if (m_selectedUuid.isEmpty()) { m_valueDisplay->append(tr("[错误] 请先选择一个特征")); return; }

    QByteArray writeData;
    switch (m_writeFormatCombo->currentIndex()) {
    case 0: writeData = parseHexInput(inputStr);
        if (writeData.isEmpty()) { m_valueDisplay->append(tr("[错误] 无效的十六进制输入")); return; }
        break;
    case 1: writeData = inputStr.toLatin1(); break;
    default: writeData = inputStr.toUtf8(); break;
    }

    if (!m_connection) return;
    const qint64 written = m_connection->write(writeData);
    m_valueDisplay->append(tr("[写入] %1 → %2 (%3/%4 bytes)")
        .arg(QString::fromUtf8(writeData.toHex(' ')).toUpper(),
             m_selectedUuid, QString::number(written), QString::number(writeData.size())));
    ++m_totalCharacteristicWrites;
    m_totalBytesWritten += static_cast<quint64>(qMax<qint64>(0, written));
    emit characteristicWritten(m_selectedUuid, written);
}

/** @brief 通知订阅切换 @param checked 是否勾选 */
void BleGattBrowser::onNotifyToggled(bool checked)
{
    if (m_selectedUuid.isEmpty()) return;
    if (checked) {
        if (!m_subscribedUuids.contains(m_selectedUuid)) m_subscribedUuids.append(m_selectedUuid);
        m_valueDisplay->append(tr("[通知] 已订阅 %1").arg(m_selectedUuid));
        if (!m_notificationTimer->isActive()) m_notificationTimer->start();
    } else {
        m_subscribedUuids.removeAll(m_selectedUuid);
        m_valueDisplay->append(tr("[通知] 已取消订阅 %1").arg(m_selectedUuid));
        if (m_subscribedUuids.isEmpty()) m_notificationTimer->stop();
    }
}

/** @brief 模拟通知数据到达 */
void BleGattBrowser::onSimulateNotification()
{
    if (m_subscribedUuids.isEmpty()) { m_notificationTimer->stop(); return; }
    const int idx = QRandomGenerator::global()->bounded(m_subscribedUuids.size());
    const QString uuid = m_subscribedUuids.at(idx);
    const int len = 2 + QRandomGenerator::global()->bounded(7);
    QByteArray data(len, 0);
    for (int i = 0; i < len; ++i) data[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
    ++m_totalNotificationsReceived;
    m_totalBytesRead += static_cast<quint64>(len);
    m_valueDisplay->append(tr("[通知] %1: %2 (%3 bytes)")
        .arg(uuid, QString::fromUtf8(data.toHex(' ')).toUpper(), QString::number(len)));
    emit notificationReceived(uuid, data);
}

/** @brief 写入格式切换 @param index 格式索引 */
void BleGattBrowser::onWriteFormatChanged(int index)
{
    static const char* hints[] = {
        QT_TRANSLATE_NOOP("BleGattBrowser", "输入写入值(十六进制，如 0A1B2C)"),
        QT_TRANSLATE_NOOP("BleGattBrowser", "输入写入值(ASCII文本)"),
        QT_TRANSLATE_NOOP("BleGattBrowser", "输入写入值(UTF-8文本)")
    };
    m_writeInput->setPlaceholderText(tr(hints[qBound(0, index, 2)]));
}

// ---- 统计信息 ----

/** @brief 获取累计服务发现次数 @return 服务发现总数 */
quint64 BleGattBrowser::totalServiceDiscoveries() const { return m_totalServiceDiscoveries; }
/** @brief 获取累计特征值读取次数 @return 特征值读取总数 */
quint64 BleGattBrowser::totalCharacteristicReads() const { return m_totalCharacteristicReads; }
/** @brief 获取累计特征值写入次数 @return 特征值写入总数 */
quint64 BleGattBrowser::totalCharacteristicWrites() const { return m_totalCharacteristicWrites; }
/** @brief 获取累计通知接收次数 @return 通知接收总数 */
quint64 BleGattBrowser::totalNotificationsReceived() const { return m_totalNotificationsReceived; }
/** @brief 获取累计描述符操作次数 @return 描述符操作总数 */
quint64 BleGattBrowser::totalDescriptorOps() const { return m_totalDescriptorOps; }
/** @brief 获取累计读取字节数 @return 读取字节总量 */
quint64 BleGattBrowser::totalBytesRead() const { return m_totalBytesRead; }
/** @brief 获取累计写入字节数 @return 写入字节总量 */
quint64 BleGattBrowser::totalBytesWritten() const { return m_totalBytesWritten; }
/** @brief 获取最后一次服务发现耗时 @return 发现耗时(毫秒) */
qint64 BleGattBrowser::lastDiscoveryDurationMs() const { return m_lastDiscoveryDurationMs; }

/** @brief 获取累计服务展开操作次数 @return 服务展开总次数 */
quint64 BleGattBrowser::totalServiceExpands() const { return m_totalServiceExpands; }

/** @brief 获取累计描述符读取次数 @return 描述符读取总次数 */
quint64 BleGattBrowser::totalDescriptorReads() const { return m_totalDescriptorReads; }

/** @brief 重置所有统计计数器 */
void BleGattBrowser::resetStatistics()
{
    m_totalServiceDiscoveries = m_totalCharacteristicReads = m_totalCharacteristicWrites = 0;
    m_totalNotificationsReceived = m_totalDescriptorOps = 0;
    m_totalBytesRead = m_totalBytesWritten = 0;
    m_lastDiscoveryDurationMs = 0;
    m_totalServiceExpands = 0;
    m_totalDescriptorReads = 0;
    m_totalConnectAttempts = 0;
}
