/**
 * @file BleGattBrowserDisplay.cpp
 * @brief GATT服务浏览器 -- 显示格式化与UI状态更新方法实现
 *
 * 本文件从 BleGattBrowser.cpp 拆分而来，包含数据显示格式化与UI状态更新相关方法:
 *   - formatHexDump:         将字节数组格式化为偏移+HEX+ASCII的经典十六进制转储
 *   - parseHexInput:         将用户输入的十六进制字符串解析为字节数组
 *   - getItemProperties:     从树控件项中提取特征属性标志位
 *   - updateButtonStates:    根据选中特征属性更新按钮启用状态
 *   - updateConnectionStatus:更新连接状态标签文本
 */

#include "connection/ble/BleGattBrowser.h"
#include "connection/ble/BleConnection.h"

// ============================================================================
// 数据格式化
// ============================================================================

/** @brief 十六进制转储格式化(偏移+HEX+ASCII)
 *  @param data 原始字节数组
 *  @return 格式化后的多行字符串，每行16字节，含偏移/HEX/ASCII三列
 */
QString BleGattBrowser::formatHexDump(const QByteArray& data) const
{
    QString r;
    for (int i = 0; i < data.size(); i += 16) {
        r += QString("%1  ").arg(i, 4, 16, QChar('0')).toUpper();
        for (int j = 0; j < 16; ++j) {
            r += (i + j < data.size())
                ? QString("%1 ").arg(static_cast<uint8_t>(data.at(i+j)), 2, 16, QChar('0')).toUpper()
                : "   ";
            if (j == 7) r += " ";
        }
        r += " |";
        for (int j = 0; j < 16 && i+j < data.size(); ++j) {
            const char c = data.at(i+j);
            r += (c >= 0x20 && c <= 0x7E) ? c : '.';
        }
        r += "|\n";
    }
    return r;
}

/** @brief 解析十六进制输入
 *  @param hexStr 用户输入的十六进制字符串(支持空格分隔和0x前缀)
 *  @return 解析后的字节数组，输入无效时可能为空
 */
QByteArray BleGattBrowser::parseHexInput(const QString& hexStr) const
{
    QString s = hexStr;
    s.remove(' ').remove("0x", Qt::CaseInsensitive);
    return QByteArray::fromHex(s.toUtf8());
}

// ============================================================================
// UI状态更新
// ============================================================================

/** @brief 从树控件项中提取特征属性标志位
 *  @param item 树控件项指针
 *  @return 特征属性标志位组合，项无效时返回空标志
 */
BleGattBrowser::CharProperties BleGattBrowser::getItemProperties(QTreeWidgetItem* item) const
{
    if (!item) return CharProperties();
    return static_cast<CharProperties>(item->data(3, Qt::UserRole).toUInt());
}

/** @brief 根据选中特征属性更新按钮启用状态
 *
 * 依据当前连接状态、是否已选中特征以及特征支持的属性标志，
 * 动态启用/禁用读取、写入、通知订阅按钮，并同步通知复选框状态。
 */
void BleGattBrowser::updateButtonStates()
{
    const bool conn = m_connection && m_connection->state() == ConnectionState::Connected;
    const bool sel = !m_selectedUuid.isEmpty();
    m_readBtn->setEnabled(conn && sel && (m_selectedProps & PropRead));
    m_writeBtn->setEnabled(conn && sel && (m_selectedProps & (PropWrite | PropWriteNoRsp)));
    m_notifyCheck->setEnabled(conn && sel && (m_selectedProps & (PropNotify | PropIndicate)));
    if (m_subscribedUuids.contains(m_selectedUuid)) {
        m_notifyCheck->blockSignals(true);
        m_notifyCheck->setChecked(true);
        m_notifyCheck->blockSignals(false);
    }
}

/** @brief 更新连接状态标签
 *
 * 根据当前BleConnection的连接状态更新状态栏文本:
 * 已连接显示设备名，连接中显示等待提示，其余显示未连接。
 */
void BleGattBrowser::updateConnectionStatus()
{
    if (!m_connection) { m_connectionStatus->setText(tr("未连接")); return; }
    switch (m_connection->state()) {
    case ConnectionState::Connected:
        m_connectionStatus->setText(tr("已连接: %1")
            .arg(m_connection->deviceName().isEmpty() ? m_connection->name() : m_connection->deviceName()));
        break;
    case ConnectionState::Connecting:
        m_connectionStatus->setText(tr("连接中...")); break;
    default:
        m_connectionStatus->setText(tr("未连接")); break;
    }
}
