/**
 * @file SerialDataLoggerExport.cpp
 * @brief 高级串口数据日志记录器 -- 格式转换导出
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 提供 exportLog() 方法，将当前日志文件转换为其他格式输出。
 * 核心逻辑见 @see SerialDataLogger.cpp
 */

#include "utils/logger2/SerialDataLogger.h"

#include <QDateTime>
#include <QFile>

/**
 * @brief 将当前日志导出为指定格式
 *
 * 读取当前日志文件内容，以目标格式重新写入 outputPath。
 * 支持 Raw/Hex/ASCII/CSV/JSON/PCAP 之间的格式转换。
 *
 * 对于 Raw/PCAP 格式的源文件，按数据块读取；
 * 对于文本格式的源文件，按行读取。
 *
 * @param outputPath 输出文件路径
 * @param format 目标输出格式
 * @return true 导出成功，false 无当前日志或文件操作失败
 */
bool SerialDataLogger::exportLog(const QString& outputPath, LogFormat format) const
{
    if (m_currentFilePath.isEmpty()) {
        return false;
    }

    QFile srcFile(m_currentFilePath);
    if (!srcFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    QFile dstFile(outputPath);
    QIODevice::OpenMode mode = QIODevice::WriteOnly;
    if (format != LogFormat::Raw && format != LogFormat::Pcap) {
        mode |= QIODevice::Text;
    }

    if (!dstFile.open(mode)) {
        srcFile.close();
        return false;
    }

    // 写入目标格式头
    if (format == LogFormat::Pcap) {
        dstFile.write(buildPcapGlobalHeader());
    } else if (format == LogFormat::Csv) {
        dstFile.write("timestamp,direction,size,data_hex,data_ascii\n");
    } else if (format == LogFormat::Json) {
        dstFile.write("{\"records\":[\n");
    }

    // 跳过源文件格式头
    if (m_format == LogFormat::Pcap) {
        srcFile.seek(24); // 跳过 PCAP 全局头
    } else if (m_format == LogFormat::Csv) {
        srcFile.readLine(); // 跳过 CSV 表头行
    } else if (m_format == LogFormat::Json) {
        srcFile.readLine(); // 跳过 {"records":[
    }

    bool firstJson = true;

    while (!srcFile.atEnd()) {
        QByteArray chunk;

        if (m_format == LogFormat::Pcap) {
            // 读取 PCAP 包头(16字节) + 数据
            QByteArray pktHdr = srcFile.read(16);
            if (pktHdr.size() < 16) {
                break;
            }
            const quint32 pktLen =
                static_cast<quint8>(pktHdr[8]) |
                (static_cast<quint32>(static_cast<quint8>(pktHdr[9])) << 8) |
                (static_cast<quint32>(static_cast<quint8>(pktHdr[10])) << 16) |
                (static_cast<quint32>(static_cast<quint8>(pktHdr[11])) << 24);
            chunk = srcFile.read(pktLen);
        } else if (m_format == LogFormat::Raw) {
            chunk = srcFile.read(4096);
        } else {
            // 文本格式: 按行读取
            QByteArray line = srcFile.readLine();
            if (line.isEmpty()) {
                continue;
            }
            chunk = line.trimmed();
        }

        if (chunk.isEmpty()) {
            continue;
        }

        // 以目标格式输出
        switch (format) {
        case LogFormat::Raw:
            dstFile.write(chunk);
            break;
        case LogFormat::Hex:
            dstFile.write(chunk.toHex(' ').toUpper() + "\n");
            break;
        case LogFormat::Ascii:
            dstFile.write(chunk + "\n");
            break;
        case LogFormat::Csv: {
            const QByteArray ts = QDateTime::currentDateTime()
                .toString(Qt::ISODateWithMs).toUtf8();
            dstFile.write(ts + ",RX," + QByteArray::number(chunk.size()) + ",");
            dstFile.write(chunk.toHex() + "," + chunk.toPercentEncoding() + "\n");
            break;
        }
        case LogFormat::Json: {
            if (!firstJson) {
                dstFile.write(",\n");
            }
            const QByteArray ts = QDateTime::currentDateTime()
                .toString(Qt::ISODateWithMs).toUtf8();
            dstFile.write("{\"ts\":\"" + ts + "\",");
            dstFile.write("\"dir\":\"RX\",\"len\":" + QByteArray::number(chunk.size()) + ",");
            dstFile.write("\"hex\":\"" + chunk.toHex() + "\"}");
            firstJson = false;
            break;
        }
        case LogFormat::Pcap:
            dstFile.write(buildPcapPacketHeader(static_cast<quint32>(chunk.size())));
            dstFile.write(chunk);
            break;
        }
    }

    // 写入目标格式尾
    if (format == LogFormat::Json) {
        dstFile.write("\n]}\n");
    }

    srcFile.close();
    dstFile.close();
    return true;
}
