/**
 * @file SerialPortProfiler.cpp
 * @brief 串口流量分析器 -- 核心实现
 *
 * 包含字节分布采集、数据包计时、突发检测、Shannon熵计算、
 * 直方图分桶、文本报告生成和文件导出。
 */

#include "serial/profiler/SerialPortProfiler.h"

#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QtMath>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 构造 / 生命周期
// ═══════════════════════════════════════════════════════════

/**
 * @brief 构造串口流量分析器
 *
 * 初始化所有统计数据为零。计时器在 startProfiling() 时启动。
 * @param parent 父对象
 */
SerialPortProfiler::SerialPortProfiler(QObject* parent)
    : QObject(parent)
    , m_byteDist{}
    , m_stats{}
{
}

// ═══════════════════════════════════════════════════════════
// 数据喂入
// ═══════════════════════════════════════════════════════════

/** @brief 喂入单个字节 -- 递增字节桶计数器，累加当前包字节数 @param byte 数据字节(0x00~0xFF) */
void SerialPortProfiler::feedByte(uint8_t byte)
{
    m_byteDist.byteCounts[byte]++;
    m_currentPacketBytes++;
    m_lastByteTime = m_timer.isValid() ? m_timer.elapsed() : 0;
    if (m_profiling) {
        m_stats.totalBytesAnalyzed++;
    }
}

/** @brief 批量喂入字节数据 -- 直接遍历原始缓冲区避免逐字节函数调用开销 @param data 字节数组 */
void SerialPortProfiler::feedBytes(const QByteArray& data)
{
    const int size = data.size();
    const char* raw = data.constData();
    for (int i = 0; i < size; ++i) {
        m_byteDist.byteCounts[static_cast<uint8_t>(raw[i])]++;
    }
    m_currentPacketBytes += size;
    m_lastByteTime = m_timer.isValid() ? m_timer.elapsed() : 0;
    if (m_profiling) {
        m_stats.totalBytesAnalyzed += static_cast<quint64>(size);
    }
}

/**
 * @brief 标记数据包边界
 *
 * 记录当前包的到达时间和大小，然后重置包计数器。
 * 当包间间隔小于平均间隔*0.5时判定为突发流量，发射 burstDetected 信号。
 * 采集状态下更新累计包统计并发射 profileUpdated 信号。
 */
void SerialPortProfiler::markPacketBoundary()
{
    if (m_currentPacketBytes <= 0) {
        return; // 空包不计入
    }

    const qint64 now = m_timer.isValid() ? m_timer.elapsed() : 0;
    m_packetTimes.append(now);
    /* 防御: quint64->int截断保护，超大包记录为INT_MAX */
    const int pktSize = (m_currentPacketBytes <= static_cast<quint64>(INT_MAX))
        ? static_cast<int>(m_currentPacketBytes)
        : INT_MAX;
    m_packetSizes.append(pktSize);

    // 突发检测: 至少两个包才能比较间隔
    if (m_packetTimes.size() >= 2) {
        const qint64 gap = now - m_packetTimes[m_packetTimes.size() - 2];
        const double threshold = burstThreshold();
        if (threshold > 0.0 && static_cast<double>(gap) < threshold) {
            m_bursts.append(qMakePair(now, pktSize));
            if (m_profiling) {
                m_stats.totalBurstsDetected++;
                emit burstDetected(now, pktSize);
            }
        }
    }

    m_currentPacketBytes = 0;
    if (m_profiling) {
        m_stats.totalPacketsAnalyzed++;
        emit profileUpdated(currentProfile());
    }
}

// ═══════════════════════════════════════════════════════════
// 采集控制
// ═══════════════════════════════════════════════════════════

/** @brief 开始流量分析采集 -- 启动计时器，重置画像数据，进入采集状态 */
void SerialPortProfiler::startProfiling()
{
    if (m_profiling) {
        return;
    }
    resetProfile();
    m_timer.start();
    m_startTime = 0;
    m_profiling = true;
}

/** @brief 停止采集 -- 更新峰值分析率和累计画像生成计数 */
void SerialPortProfiler::stopProfiling()
{
    if (!m_profiling) {
        return;
    }
    m_profiling = false;
    // 更新峰值分析率
    const qint64 elapsed = m_timer.elapsed();
    if (elapsed > 0) {
        const double rate = static_cast<double>(m_stats.totalBytesAnalyzed) * 1000.0
                            / static_cast<double>(elapsed);
        if (rate > m_stats.peakAnalysisRate) {
            m_stats.peakAnalysisRate = rate;
        }
    }
    m_stats.totalProfilesGenerated++;
}

/** @brief 重置当前画像数据(不影响累计统计) -- 清空字节分布、包时间/大小、突发事件 */
void SerialPortProfiler::resetProfile()
{
    m_byteDist = ByteDistribution{};
    m_packetTimes.clear();
    m_packetSizes.clear();
    m_bursts.clear();
    m_lastByteTime = 0;
    m_currentPacketBytes = 0;
    m_startTime = 0;
}

/** @brief 查询是否正在采集 @return true 表示正在分析中 */
bool SerialPortProfiler::isProfiling() const
{
    return m_profiling;
}

// ═══════════════════════════════════════════════════════════
// 查询
// ═══════════════════════════════════════════════════════════

/**
 * @brief 获取当前实时流量画像
 *
 * 聚合所有已采集数据为 TrafficProfile 结构体: Shannon熵、平均包大小、
 * 平均到达间隔、峰值吞吐率和压缩比(基于 (8-entropy)/8 )。
 * @return TrafficProfile快照
 */
TrafficProfile SerialPortProfiler::currentProfile() const
{
    TrafficProfile profile;
    // 总字节数: 从字节分布中求和
    quint64 totalBytes = 0;
    for (int i = 0; i < 256; ++i) {
        totalBytes += static_cast<quint64>(m_byteDist.byteCounts[i]);
    }
    profile.totalBytes = totalBytes;
    profile.totalPackets = static_cast<quint64>(m_packetSizes.size());

    // 平均包大小
    if (!m_packetSizes.isEmpty()) {
        qint64 sumSizes = 0;
        for (int sz : m_packetSizes) {
            sumSizes += sz;
        }
        profile.avgPacketSize = static_cast<double>(sumSizes) / static_cast<double>(m_packetSizes.size());
    }
    // 平均包间到达间隔
    if (m_packetTimes.size() >= 2) {
        qint64 totalGap = 0;
        for (int i = 1; i < m_packetTimes.size(); ++i) {
            totalGap += (m_packetTimes[i] - m_packetTimes[i - 1]);
        }
        profile.avgInterArrivalMs = static_cast<double>(totalGap)
                                    / static_cast<double>(m_packetTimes.size() - 1);
    }
    // 峰值吞吐率(整个分析周期的平均吞吐)
    if (m_packetTimes.size() >= 2 && m_timer.isValid()) {
        const qint64 duration = m_timer.elapsed();
        if (duration > 0) {
            profile.peakRateBytesPerSec = static_cast<double>(totalBytes) * 1000.0
                                          / static_cast<double>(duration);
        }
    }
    // 持续时间
    if (m_timer.isValid()) {
        profile.profileDurationMs = m_timer.elapsed();
    }
    // 字节分布 + 熵 + 最常见/最少见字节
    profile.byteDist = m_byteDist;
    profile.byteDist.entropy = calculateEntropy();
    int maxCount = 0, minCount = std::numeric_limits<int>::max();
    for (int i = 0; i < 256; ++i) {
        if (m_byteDist.byteCounts[i] > maxCount) {
            maxCount = m_byteDist.byteCounts[i];
            profile.byteDist.mostCommonByte = i;
        }
        if (m_byteDist.byteCounts[i] < minCount) {
            minCount = m_byteDist.byteCounts[i];
            profile.byteDist.leastCommonByte = i;
        }
    }
    // 压缩比: 基于(8.0 - entropy) / 8.0，熵越低越可压缩
    if (profile.byteDist.entropy > 0.0) {
        profile.compressionRatio = qMax(0.0, (8.0 - profile.byteDist.entropy) / 8.0);
    }
    return profile;
}

/** @brief 获取当前字节值分布统计(含熵和最常见/最少见字节) @return ByteDistribution快照 */
ByteDistribution SerialPortProfiler::byteDistribution() const
{
    ByteDistribution dist = m_byteDist;
    dist.entropy = calculateEntropy();
    int maxCount = 0, minCount = std::numeric_limits<int>::max();
    for (int i = 0; i < 256; ++i) {
        if (m_byteDist.byteCounts[i] > maxCount) {
            maxCount = m_byteDist.byteCounts[i];
            dist.mostCommonByte = i;
        }
        if (m_byteDist.byteCounts[i] < minCount) {
            minCount = m_byteDist.byteCounts[i];
            dist.leastCommonByte = i;
        }
    }
    return dist;
}

/**
 * @brief 包间到达间隔直方图
 *
 * 将所有包间间隔分桶统计，返回每桶的下界(ms)和频次。
 * @param bins 分桶数量(默认20)
 * @return 直方图数据
 */
QVector<QPair<qint64, int>> SerialPortProfiler::interArrivalHistogram(int bins) const
{
    QVector<QPair<qint64, int>> histogram;
    if (m_packetTimes.size() < 2 || bins <= 0) {
        return histogram;
    }
    // 计算间隔
    QVector<qint64> gaps;
    gaps.reserve(m_packetTimes.size() - 1);
    for (int i = 1; i < m_packetTimes.size(); ++i) {
        gaps.append(m_packetTimes[i] - m_packetTimes[i - 1]);
    }
    qint64 minGap = *std::min_element(gaps.begin(), gaps.end());
    qint64 maxGap = *std::max_element(gaps.begin(), gaps.end());
    if (minGap == maxGap) {
        histogram.append(qMakePair(minGap, gaps.size()));
        return histogram;
    }
    const double binWidth = static_cast<double>(maxGap - minGap) / static_cast<double>(bins);
    histogram.resize(bins);
    for (int i = 0; i < bins; ++i) {
        histogram[i] = qMakePair(minGap + static_cast<qint64>(binWidth * i), 0);
    }
    for (qint64 gap : gaps) {
        int idx = qMin(static_cast<int>((static_cast<double>(gap - minGap)) / binWidth), bins - 1);
        histogram[idx].second++;
    }
    return histogram;
}

/**
 * @brief 数据包大小直方图
 *
 * 将所有包大小分桶统计，返回每桶的大小下界(bytes)和频次。
 * @param bins 分桶数量(默认20)
 * @return 直方图数据
 */
QVector<QPair<int, int>> SerialPortProfiler::packetSizeHistogram(int bins) const
{
    QVector<QPair<int, int>> histogram;
    if (m_packetSizes.isEmpty() || bins <= 0) {
        return histogram;
    }
    int minSize = *std::min_element(m_packetSizes.begin(), m_packetSizes.end());
    int maxSize = *std::max_element(m_packetSizes.begin(), m_packetSizes.end());
    if (minSize == maxSize) {
        histogram.append(qMakePair(minSize, m_packetSizes.size()));
        return histogram;
    }
    const double binWidth = static_cast<double>(maxSize - minSize) / static_cast<double>(bins);
    histogram.resize(bins);
    for (int i = 0; i < bins; ++i) {
        histogram[i] = qMakePair(minSize + static_cast<int>(binWidth * i), 0);
    }
    for (int sz : m_packetSizes) {
        int idx = qMin(static_cast<int>((static_cast<double>(sz - minSize)) / binWidth), bins - 1);
        histogram[idx].second++;
    }
    return histogram;
}

/** @brief 获取突发流量时间线 @return 突发事件列表(时间戳ms, 突发字节数) */
QVector<QPair<qint64, int>> SerialPortProfiler::burstTimeline() const
{
    return m_bursts;
}

// ═══════════════════════════════════════════════════════════
// 报告
// ═══════════════════════════════════════════════════════════

/**
 * @brief 生成文本格式的流量分析报告
 *
 * 包含: 基本信息、吞吐率、字节分布统计、Shannon熵与压缩比、
 * 包大小分布、到达间隔分布、突发流量检测、Top-10高频字节。
 * @return 完整报告文本
 */
QString SerialPortProfiler::generateTextReport()
{
    const TrafficProfile profile = currentProfile();
    QString report;
    QTextStream s(&report);

    // ── 报告头 ──
    s << "═══════════════════════════════════════════════\n"
      << "        EmbedDebug 串口流量分析报告\n"
      << "═══════════════════════════════════════════════\n\n";

    // ── 基本统计 ──
    s << "── 基本统计 ──────────────────────────────\n"
      << "  分析时长:       " << profile.profileDurationMs << " ms\n"
      << "  总字节数:       " << profile.totalBytes << "\n"
      << "  总数据包数:     " << profile.totalPackets << "\n"
      << "  平均包大小:     " << QString::number(profile.avgPacketSize, 'f', 1) << " bytes\n"
      << "  平均包间间隔:   " << QString::number(profile.avgInterArrivalMs, 'f', 2) << " ms\n"
      << "  峰值吞吐率:     " << QString::number(profile.peakRateBytesPerSec, 'f', 1) << " bytes/s\n\n";

    // ── 字节分布 ──
    s << "── 字节分布统计 ──────────────────────────\n"
      << "  Shannon 熵:     " << QString::number(profile.byteDist.entropy, 'f', 4) << " bits\n"
      << "  最大熵(均匀):   8.0000 bits\n"
      << "  可压缩比:       " << QString::number(profile.compressionRatio * 100.0, 'f', 1) << "%\n"
      << "  最常见字节:     0x" << QString::number(profile.byteDist.mostCommonByte, 16).toUpper().rightJustified(2, '0')
      << " (" << m_byteDist.byteCounts[profile.byteDist.mostCommonByte] << " 次)\n"
      << "  最少见字节:     0x" << QString::number(profile.byteDist.leastCommonByte, 16).toUpper().rightJustified(2, '0')
      << " (" << m_byteDist.byteCounts[profile.byteDist.leastCommonByte] << " 次)\n\n";

    // ── Top-10 高频字节 ──
    s << "── Top-10 高频字节 ────────────────────────\n";
    QVector<QPair<int, int>> sortedBytes; // (count, byte)
    sortedBytes.reserve(256);
    for (int i = 0; i < 256; ++i) {
        if (m_byteDist.byteCounts[i] > 0) {
            sortedBytes.append(qMakePair(m_byteDist.byteCounts[i], i));
        }
    }
    std::sort(sortedBytes.begin(), sortedBytes.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });
    const int topN = qMin(10, sortedBytes.size());
    for (int i = 0; i < topN; ++i) {
        const int byteVal = sortedBytes[i].second;
        const char ascii = (byteVal >= 0x20 && byteVal <= 0x7E) ? static_cast<char>(byteVal) : '.';
        s << "  #" << QString::number(i + 1).rightJustified(2)
          << "  0x" << QString::number(byteVal, 16).toUpper().rightJustified(2, '0')
          << " '" << ascii << "'"
          << "  " << QString::number(sortedBytes[i].first).rightJustified(8) << " 次\n";
    }
    s << "\n";

    // ── 包大小分布 ──
    if (!m_packetSizes.isEmpty()) {
        s << "── 数据包大小分布 ────────────────────────\n";
        for (const auto& bucket : packetSizeHistogram(10)) {
            if (bucket.second > 0) {
                s << "  " << QString::number(bucket.first).rightJustified(6) << " bytes"
                  << "  " << QString::number(bucket.second).rightJustified(5) << " 次\n";
            }
        }
        s << "\n";
    }

    // ── 到达间隔分布 ──
    if (m_packetTimes.size() >= 2) {
        s << "── 包间到达间隔分布 ──────────────────────\n";
        for (const auto& bucket : interArrivalHistogram(10)) {
            if (bucket.second > 0) {
                s << "  " << QString::number(bucket.first).rightJustified(8) << " ms"
                  << "  " << QString::number(bucket.second).rightJustified(5) << " 次\n";
            }
        }
        s << "\n";
    }

    // ── 突发流量 ──
    s << "── 突发流量检测 ──────────────────────────\n"
      << "  检测到的突发次数: " << m_bursts.size() << "\n";
    if (!m_bursts.isEmpty()) {
        s << "  突发阈值:         " << QString::number(burstThreshold(), 'f', 2) << " ms\n";
        const int burstShow = qMin(10, m_bursts.size());
        for (int i = 0; i < burstShow; ++i) {
            s << "  T+" << QString::number(m_bursts[i].first).rightJustified(8) << " ms"
              << "  " << QString::number(m_bursts[i].second).rightJustified(5) << " bytes\n";
        }
        if (m_bursts.size() > 10) {
            s << "  ... 还有 " << (m_bursts.size() - 10) << " 个突发事件\n";
        }
    }
    s << "\n═══════════════════════════════════════════════\n"
      << "  报告生成时间: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n"
      << "═══════════════════════════════════════════════\n";
    s.flush();
    return report;
}

/**
 * @brief 将分析报告导出到文件
 *
 * 生成文本报告并写入指定路径，成功时发射 reportGenerated 信号并更新累计导出统计。
 * @param filePath 目标文件路径
 * @return true 表示导出成功
 */
bool SerialPortProfiler::exportReport(const QString& filePath)
{
    const QString content = generateTextReport();
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << content;
    out.flush();
    file.close();
    m_stats.totalReportsExported++;
    emit reportGenerated(filePath);
    return true;
}

// ═══════════════════════════════════════════════════════════
// 内部计算
// ═══════════════════════════════════════════════════════════

/**
 * @brief 从字节分布计算Shannon信息熵
 *
 * H = -sum(p_i * log2(p_i))，其中 p_i = count_i / total
 * 完全均匀分布时 H=8.0 bits，单一字节时 H=0.0 bits。
 * @return 熵值(0.0~8.0 bits)
 */
double SerialPortProfiler::calculateEntropy() const
{
    quint64 total = 0;
    for (int i = 0; i < 256; ++i) {
        total += static_cast<quint64>(m_byteDist.byteCounts[i]);
    }
    if (total == 0) {
        return 0.0;
    }
    double entropy = 0.0;
    for (int i = 0; i < 256; ++i) {
        if (m_byteDist.byteCounts[i] > 0) {
            const double p = static_cast<double>(m_byteDist.byteCounts[i]) / static_cast<double>(total);
            entropy -= p * qLn(p);
        }
    }
    return entropy / qLn(2.0); // 自然对数转log2
}

/**
 * @brief 估算突发阈值 -- 平均包间到达间隔的50%
 *
 * 至少需要2个包才能计算，数据不足时返回0。
 * @return 间隔阈值(ms)
 */
double SerialPortProfiler::burstThreshold() const
{
    if (m_packetTimes.size() < 2) {
        return 0.0;
    }
    qint64 totalGap = 0;
    for (int i = 1; i < m_packetTimes.size(); ++i) {
        totalGap += (m_packetTimes[i] - m_packetTimes[i - 1]);
    }
    return static_cast<double>(totalGap) / static_cast<double>(m_packetTimes.size() - 1) * 0.5;
}
