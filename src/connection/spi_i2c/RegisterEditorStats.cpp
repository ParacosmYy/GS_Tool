/**
 * @file RegisterEditorStats.cpp
 * @brief 寄存器编辑器 - 日志格式化与统计接口实现
 *
 * 从 RegisterEditor.cpp 拆分而来，包含日志追加、HEX格式化、
 * 所有统计getter和resetStatistics方法。
 */

#include "connection/spi_i2c/RegisterEditor.h"

/** @brief 追加带颜色编码的日志消息(TX蓝色/RX绿色) @param msg 日志消息 @param isTx true=发送(TX)，false=接收(RX) */
void RegisterEditor::appendLog(const QString& msg, bool isTx)
{
    if (!m_log) return;
    /// TX用Link色，RX用ToolTipText色，跟随QSS主题
    QString color = isTx ? palette().color(QPalette::Link).name()
                         : palette().color(QPalette::ToolTipText).name();
    m_log->append(QString("<span style='color:%1'>%2</span>").arg(color, msg));
}

/** @brief 格式化字节数组为十六进制字符串 @param data 字节数组 @return "0x1A 0x2B"格式字符串 */
QString RegisterEditor::formatHex(const QByteArray& data)
{
    QStringList hexParts;
    for (char byte : data) {
        hexParts.append(QString("0x%1").arg(
            static_cast<quint8>(byte), 2, 16, QChar('0')).toUpper());
    }
    return hexParts.join(" ");
}

/** @brief 获取会话级读操作次数 @return 累计读操作计数 */
int RegisterEditor::readCount() const
{
    return m_readCount;
}

/** @brief 获取会话级写操作次数 @return 累计写操作计数 */
int RegisterEditor::writeCount() const
{
    return m_writeCount;
}

/** @brief 导出操作日志为纯文本 @return 日志文本内容 */
QString RegisterEditor::exportLog() const
{
    if (!m_log) return QString();
    return m_log->toPlainText();
}

/** @brief 获取累计寄存器读取次数 @return 读取总数 */
quint64 RegisterEditor::totalRegisterReads() const
{
    return m_totalRegisterReads;
}

/** @brief 获取累计寄存器写入次数 @return 写入总数 */
quint64 RegisterEditor::totalRegisterWrites() const
{
    return m_totalRegisterWrites;
}

/** @brief 获取平均寄存器访问耗时(ms) @return 平均访问时间 */
double RegisterEditor::avgAccessTimeMs() const
{
    const quint64 totalOps = m_totalRegisterReads + m_totalRegisterWrites;
    if (totalOps == 0) return 0.0;
    return static_cast<double>(m_totalAccessTimeUs) / static_cast<double>(totalOps) / 1000.0;
}

/** @brief 重置所有统计计数器 */
void RegisterEditor::resetStatistics()
{
    m_totalRegisterReads = 0;
    m_totalRegisterWrites = 0;
    m_totalLogClears = 0;
    m_totalErrors = 0;
    m_accessedAddresses.clear();
    m_totalAccessTimeUs = 0;
}
