/**
 * @file DataChecksumVerifier.cpp
 * @brief 数据校验验证引擎 -- 验证接口与CSV导出
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 从DataChecksumVerifier.cpp拆分: 构造函数 + verify/verifyAll/
 * computeBytes/exportResults。算法核心见DataChecksumVerifierAlgo.cpp，
 * 统计/元数据见DataChecksumVerifierStats.cpp。
 */

#include "utils/checksum/DataChecksumVerifier.h"

#include <QFile>
#include <QTextStream>

// ─────────────────────────── 构造函数 ───────────────────────────

/** @brief 构造函数，初始化统计结构 @param parent 父对象 */
DataChecksumVerifier::DataChecksumVerifier(QObject *parent)
    : QObject(parent)
{
    resetStatistics();
}

// ─────────────────────────── 验证接口 ───────────────────────────

/**
 * @brief 验证单算法校验和，记录统计并发射信号
 * @param algo 算法
 * @param data 数据
 * @param expectedValue 期望值
 * @param startPos 起始偏移
 * @param length 数据长度
 * @return 验证结果(含计算耗时)
 */
DataChecksumVerifier::VerificationResult DataChecksumVerifier::verify(
    Algorithm algo, const QByteArray &data, quint64 expectedValue,
    int startPos, int length)
{
    QElapsedTimer timer;
    timer.start();

    quint64 computed = compute(algo, data, startPos, length);

    VerificationResult result;
    result.algorithm = algo;
    result.expectedValue = expectedValue;
    result.computedValue = computed;
    result.match = (computed == expectedValue);
    result.dataLength = 0;

    if (!data.isEmpty()) {
        int start = qBound(0, startPos, data.size() - 1);
        result.dataLength = (length < 0) ? (data.size() - start)
                                         : qMin(length, data.size() - start);
    }

    result.computationTimeUs = static_cast<double>(timer.nsecsElapsed()) / 1000.0;

    ++m_stats.totalVerifications;
    if (result.match)
        ++m_stats.totalPasses;
    else
        ++m_stats.totalFailures;
    m_stats.totalBytesProcessed += static_cast<quint64>(qMax(0, result.dataLength));
    m_stats.computationsByAlgorithm[static_cast<int>(algo)]++;

    double totalTime = m_stats.avgComputationTimeUs
                       * static_cast<double>(m_stats.totalVerifications - 1);
    m_stats.avgComputationTimeUs =
        (totalTime + result.computationTimeUs) / static_cast<double>(m_stats.totalVerifications);

    emit verificationComplete(result);
    return result;
}

/**
 * @brief 对同一数据运行全部17种算法验证
 * @param data 数据
 * @param expectedValue 期望值
 * @param startPos 起始偏移
 * @param length 数据长度
 * @return 每种算法的验证结果列表
 */
QList<DataChecksumVerifier::VerificationResult> DataChecksumVerifier::verifyAll(
    const QByteArray &data, quint64 expectedValue, int startPos, int length)
{
    QList<VerificationResult> results;
    results.reserve(17);
    for (Algorithm algo : allAlgorithms())
        results.append(verify(algo, data, expectedValue, startPos, length));
    emit allVerificationsComplete(results);
    return results;
}

/**
 * @brief 计算校验和并返回原始字节(大端序)
 * @param algo 算法
 * @param data 数据
 * @param startPos 起始偏移
 * @param length 数据长度
 * @return 校验和的原始字节
 */
QByteArray DataChecksumVerifier::computeBytes(Algorithm algo, const QByteArray &data,
                                              int startPos, int length)
{
    quint64 value = compute(algo, data, startPos, length);
    int width = algorithmWidth(algo);
    int bytes = width / 8;
    QByteArray result(bytes, Qt::Uninitialized);
    for (int i = 0; i < bytes; ++i)
        result[i] = static_cast<char>((value >> (8 * (bytes - 1 - i))) & 0xFF);
    return result;
}

/**
 * @brief 导出验证结果到CSV文件
 * @param filePath 文件路径
 * @param results 验证结果列表
 * @return true=成功写入
 */
bool DataChecksumVerifier::exportResults(const QString &filePath,
                                         const QList<VerificationResult> &results)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);

    stream << QStringLiteral("Algorithm,Expected,Computed,Match,DataLength,TimeUs\n");

    for (const auto &r : results) {
        stream << algorithmName(r.algorithm)
               << QLatin1Char(',')
               << QStringLiteral("0x") << QString::number(r.expectedValue, 16).toUpper()
               << QLatin1Char(',')
               << QStringLiteral("0x") << QString::number(r.computedValue, 16).toUpper()
               << QLatin1Char(',')
               << (r.match ? QLatin1String("PASS") : QLatin1String("FAIL"))
               << QLatin1Char(',')
               << r.dataLength
               << QLatin1Char(',')
               << QString::number(r.computationTimeUs, 'f', 3)
               << QLatin1Char('\n');
    }

    file.close();
    return true;
}

// ── 算法核心见 DataChecksumVerifierAlgo.cpp ──
// ── 统计/元数据见 DataChecksumVerifierStats.cpp ──
