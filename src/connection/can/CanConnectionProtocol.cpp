/**
 * @file CanConnectionProtocol.cpp
 * @brief CAN连接 — DBC加载/信号解码/命令发送/接收解析(拆分自CanConnection.cpp)
 *
 * 实现:
 *   - loadDbcFile: 加载DBC数据库文件用于信号解码
 *   - decodeFrameSignals: 解码CAN帧中的信号值(依赖已加载的DBC)
 *   - dbcParser: 获取DBC解析器实例
 *   - sendCommand: 向适配器发送LAWICEL原始命令
 *   - onSerialDataReceived: 串口数据接收槽函数
 *   - parseBuffer: 解析接收缓冲区中的LAWICEL帧
 */

#include "connection/can/CanConnection.h"
#include "connection/can/CanFrameParser.h"
#include "protocol/can/DbcParser.h"

/** @brief 加载DBC数据库文件用于信号解码 @param filePath DBC文件路径 @return 加载成功返回true */
bool CanConnection::loadDbcFile(const QString& filePath)
{
    if (!m_dbcParser) {
        m_dbcParser = new DbcParser(this);
    }

    bool ok = m_dbcParser->loadFromFile(filePath);
    if (ok) {
        emit dbcLoaded(true, m_dbcParser->messageCount());
    } else {
        emit dbcLoaded(false, 0);
    }
    return ok;
}

/** @brief 解码CAN帧中的信号值(依赖已加载的DBC) @param id 帧ID @param data 帧数据 @return 信号名→物理值映射 */
QMap<QString, double> CanConnection::decodeFrameSignals(quint32 id, const QByteArray& data) const
{
    if (!m_dbcParser) {
        return {};
    }

    QMap<QString, double> result = m_dbcParser->decodeFrame(id, data);
    m_totalSignalsDecoded += static_cast<quint64>(result.size());
    return result;
}

/** @brief 获取DBC解析器实例 @return DBC解析器指针，未加载时为nullptr */
DbcParser* CanConnection::dbcParser() const
{
    return m_dbcParser;
}

/** @brief 向适配器发送LAWICEL原始命令 @param cmd 命令字符串(不含结尾\r) @return 实际写入字节数，失败返回-1 */
qint64 CanConnection::sendCommand(const QString& cmd)
{
    if (!m_serialPort) return -1;
    QByteArray payload = cmd.toUtf8() + '\r';
    return m_serialPort->write(payload);
}

/** @brief 串口数据接收槽函数，追加到接收缓冲区并触发解析 @param data 从串口接收到的原始字节 */
void CanConnection::onSerialDataReceived(const QByteArray& data)
{
    m_rxBuffer.append(data);
    parseBuffer();
}

/** @brief 解析接收缓冲区中的LAWICEL帧，提取有效CAN帧并发射信号 */
void CanConnection::parseBuffer()
{
    /* LAWICEL帧以\r分隔，z表示ACK，\a表示错误 */
    while (m_rxBuffer.contains('\r')) {
        int idx = m_rxBuffer.indexOf('\r');
        QByteArray line = m_rxBuffer.left(idx).trimmed();
        m_rxBuffer.remove(0, idx + 1);

        if (line.isEmpty() || line == "z") continue;  // ACK或空行
        if (line.startsWith('\a')) {
            emit errorOccurred(tr("CAN适配器报告错误"));
            ++m_totalErrors;
            ++m_totalErrorFrames;
            continue;
        }

        /* 尝试解析为CAN帧 */
        CanFrameParser parser;
        CanFrame frame = parser.parseFrame(line);
        if (frame.dlc > 0 || frame.rtr) {

            /* 帧过滤检查 */
            if (!acceptsFilter(frame.id, frame.extended)) {
                ++m_totalFramesFiltered;
                continue;
            }

            /* 更新帧类型统计 */
            if (frame.extended) {
                ++m_totalExtendedFrames;
            } else {
                ++m_totalStandardFrames;
            }
            if (frame.rtr) {
                ++m_totalRtrFrames;
            }

            /// 更新统计: 接收到有效CAN帧
            ++m_totalFramesReceived;
            m_totalBytesReceived += static_cast<quint64>(frame.data.size());

            /// 计算peakFramesPerSec: 每秒边界检查
            ++m_lastSecFrameCount;
            qint64 currentSec = m_peakFpsTimer.elapsed() / 1000;
            if (currentSec > m_lastPeakSec) {
                if (m_lastSecFrameCount > m_peakFramesPerSec) {
                    m_peakFramesPerSec = m_lastSecFrameCount;
                }
                m_lastSecFrameCount = 0;
                m_lastPeakSec = currentSec;
            }

            if (frame.fd) {
                emit fdFrameReceived(static_cast<int>(frame.id),
                                     frame.data, frame.extended);
            } else {
                emit frameReceived(static_cast<int>(frame.id),
                                   frame.data, frame.extended, frame.rtr);
            }
            emit dataReceived(line);  // 也向上层转发原始数据
        } else if (!line.isEmpty()) {
            ++m_totalFrameErrors;  ///< 帧解析失败(无法识别的帧格式)
            ++m_totalDroppedFrames; ///< 累计丢帧数(解析失败)
        }
    }
}

/** @brief 根据波特率获取LAWICEL S命令编号 @param bitrate 波特率 @return LAWICEL命令字符串 */
QString CanConnection::bitrateToCommand(int bitrate)
{
    if (bitrate >= 1000000)      return "S8";
    else if (bitrate >= 800000)  return "S7";
    else if (bitrate >= 500000)  return "S6";
    else if (bitrate >= 250000)  return "S5";
    else if (bitrate >= 125000)  return "S4";
    else if (bitrate >= 100000)  return "S3";
    else if (bitrate >= 50000)   return "S2";
    else                         return "S1";
}

/** @brief 重置所有统计计数器 */
void CanConnection::resetStats()
{
    m_totalFramesSent = 0;
    m_totalFramesReceived = 0;
    m_totalBytesSent = 0;
    m_totalBytesReceived = 0;
    m_totalErrors = 0;
    m_totalStandardFrames = 0;
    m_totalExtendedFrames = 0;
    m_totalRtrFrames = 0;
    m_totalErrorFrames = 0;
    m_totalFrameErrors = 0;
    m_totalFramesFiltered = 0;
    m_totalSignalsDecoded = 0;
    m_totalBusOffEvents = 0;
    m_totalFiltersActive = 0;
    m_totalDroppedFrames = 0;
    m_peakFramesPerSec = 0;
    m_lastSecFrameCount = 0;
    m_lastPeakSec = 0;
    m_peakFpsTimer.restart();
}
