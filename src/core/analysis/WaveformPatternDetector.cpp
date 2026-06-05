/**
 * @file WaveformPatternDetector.cpp
 * @brief 波形模式检测器实现 -- 在线统计与自相关分析
 *
 * 使用Welford算法在线更新均值/方差，自相关函数检测周期特征，
 * 阈值判定检测毛刺/跌落/电平跳变。
 */

#include "core/analysis/WaveformPatternDetector.h"

#include <QtMath>
#include <algorithm>

// ============================================================================
// 构造 / 配置
// ============================================================================

/** @brief 构造函数，初始化检测器和在线统计状态 @param parent 父对象 */
WaveformPatternDetector::WaveformPatternDetector(QObject* parent)
    : QObject(parent)
    , m_sampleIndex(0)
    , m_runningMean(0.0)
    , m_runningM2(0.0)
    , m_runningCount(0)
{
}

/** @brief 设置检测配置参数，新配置立即生效 @param config 新的检测配置 */
void WaveformPatternDetector::setConfig(const DetectionConfig& config)
{
    m_config = config;
}

/** @brief 获取当前检测配置 @return 配置常量引用 */
const WaveformPatternDetector::DetectionConfig& WaveformPatternDetector::config() const
{
    return m_config;
}

// ============================================================================
// 数据输入
// ============================================================================

/** @brief 喂入单个采样值 -- 追加到环形缓冲区并在线更新统计量
 *  @param value 新采样值
 *
 *  环形缓冲区大小由analysisWindowSize控制。当缓冲区满时，
 *  移除最早的数据以保持窗口大小恒定。使用Welford算法在线更新均值和方差。
 */
void WaveformPatternDetector::feedSample(double value)
{
    /* 追加到缓冲区 */
    m_buffer.append(value);
    ++m_sampleIndex;
    ++m_stats.totalSamplesProcessed;

    /* 维持环形窗口大小 */
    while (m_buffer.size() > m_config.analysisWindowSize) {
        m_buffer.removeFirst();
    }

    /* 在线统计更新(Welford) */
    updateRunningStats(value);
}

/** @brief 批量喂入采样值 @param samples 采样值向量 */
void WaveformPatternDetector::feedSamples(const QVector<double>& samples)
{
    for (double s : samples) {
        feedSample(s);
    }
}

// ============================================================================
// 在线统计 (Welford算法)
// ============================================================================

/** @brief 在线更新均值和方差 -- Welford算法，数值稳定
 *  @param value 新采样值
 *
 *  维护m_runningMean(均值)和m_runningM2(辅助方差)，
 *  标准差可由sqrt(M2 / (count-1))计算。
 */
void WaveformPatternDetector::updateRunningStats(double value)
{
    ++m_runningCount;
    double delta = value - m_runningMean;
    m_runningMean += delta / static_cast<double>(m_runningCount);
    double delta2 = value - m_runningMean;
    m_runningM2 += delta * delta2;
}

// ============================================================================
// 分析入口
// ============================================================================

/** @brief 执行分析 -- 运行所有已启用的检测算法，发射检测到的模式
 *
 *  依次调用: 毛刺检测 → 跌落检测 → 周期检测 → 电平跳变检测。
 *  每个检测算法独立运行，结果追加到m_patterns。
 *  分析完成后发射analysisComplete信号。
 */
void WaveformPatternDetector::analyze()
{
    /* 缓冲区数据不足时跳过 */
    if (m_buffer.size() < m_config.minPatternLength) {
        emit analysisComplete(0);
        return;
    }

    /* 清空上次检测结果，防止无界累积 */
    m_patterns.clear();
    int patternsBefore = 0;

    /* 按配置顺序运行各检测算法 */
    if (m_config.detectSpikes) {
        detectSpikes();
    }
    if (m_config.detectDropouts) {
        detectDropouts();
    }
    if (m_config.detectPeriodic) {
        detectPeriodicPatterns();
    }

    /* 电平跳变始终检测(核心信号特征) */
    detectLevelChanges();

    int newPatterns = m_patterns.size() - patternsBefore;
    ++m_stats.totalAnalysisRuns;

    /* 更新统计: 平均置信度 */
    if (!m_patterns.isEmpty()) {
        double confSum = 0.0;
        for (const auto& p : m_patterns) {
            confSum += p.confidence;
        }
        m_stats.avgConfidence = confSum / static_cast<double>(m_patterns.size());
    }

    emit analysisComplete(newPatterns);
}

// ============================================================================
// 毛刺检测
// ============================================================================

/** @brief 毛刺检测 -- 查找超出spikeThreshold * stddev的异常采样点
 *
 *  遍历缓冲区，将偏离均值超过spikeThreshold倍标准差的连续区域标记为毛刺。
 *  连续的异常点合并为一个毛刺模式，置信度基于偏离程度计算。
 */
void WaveformPatternDetector::detectSpikes()
{
    if (m_runningCount < 2) return;

    double stddev = qSqrt(m_runningM2 / static_cast<double>(m_runningCount - 1));
    if (stddev < 1e-12) return; // 信号为常数，无毛刺

    double threshold = m_config.spikeThreshold * stddev;
    int i = 0;
    int bufSize = m_buffer.size();

    while (i < bufSize) {
        double deviation = qAbs(m_buffer[i] - m_runningMean);
        if (deviation > threshold) {
            /* 找到毛刺起始点 */
            int start = i;
            double peakDeviation = deviation;

            /* 扩展到连续异常区域结束 */
            while (i < bufSize && qAbs(m_buffer[i] - m_runningMean) > threshold) {
                double curDev = qAbs(m_buffer[i] - m_runningMean);
                if (curDev > peakDeviation) {
                    peakDeviation = curDev;
                }
                ++i;
            }
            int end = i - 1;

            /* 忽略过短的模式 */
            if ((end - start + 1) < m_config.minPatternLength) {
                /* 单点毛刺也算，不强制minPatternLength */
            }

            /* 计算置信度: 偏离越大置信度越高 */
            double confidence = qBound(0.0, qMin(1.0, peakDeviation / (threshold * 2.0)), 1.0);

            DetectedPattern pattern;
            pattern.type = PatternType::Spike;
            pattern.startIndex = start;
            pattern.endIndex = end;
            pattern.amplitude = peakDeviation;
            pattern.periodMs = 0.0;
            pattern.confidence = confidence;
            pattern.description = tr("毛刺: 索引%1~%2, 偏离%.2fσ, 置信度%.0f%%")
                .arg(start).arg(end)
                .arg(peakDeviation / stddev)
                .arg(confidence * kConfidenceScale);

            m_patterns.append(pattern);
            ++m_stats.totalSpikesFound;
            ++m_stats.totalPatternsDetected;

            if (confidence > m_stats.peakConfidence) {
                m_stats.peakConfidence = confidence;
            }

            emit patternDetected(pattern);
        } else {
            ++i;
        }
    }
}

// ============================================================================
// 跌落检测
// ============================================================================

/** @brief 跌落检测 -- 查找信号降至dropoutThreshold * mean以下的区域
 *
 *  遍历缓冲区，将低于均值一定比例的连续区域标记为跌落。
 *  适用于检测信号断路、接触不良等导致的信号丢失。
 */
void WaveformPatternDetector::detectDropouts()
{
    if (m_runningCount < 2) return;

    double absMean = qAbs(m_runningMean);
    if (absMean < 1e-12) return; // 零均值信号无法判断跌落

    double threshold = m_config.dropoutThreshold * absMean;
    int i = 0;
    int bufSize = m_buffer.size();

    while (i < bufSize) {
        if (qAbs(m_buffer[i]) < threshold) {
            int start = i;
            double minVal = qAbs(m_buffer[i]);

            /* 扩展到连续跌落区域结束 */
            while (i < bufSize && qAbs(m_buffer[i]) < threshold) {
                double curAbs = qAbs(m_buffer[i]);
                if (curAbs < minVal) {
                    minVal = curAbs;
                }
                ++i;
            }
            int end = i - 1;

            /* 长度检查 */
            if ((end - start + 1) < m_config.minPatternLength) {
                continue;
            }

            /* 置信度: 跌落越深置信度越高 */
            double dropRatio = 1.0 - (minVal / absMean);
            double confidence = qBound(0.0, dropRatio / m_config.dropoutThreshold * 0.5, 1.0);

            DetectedPattern pattern;
            pattern.type = PatternType::Dropout;
            pattern.startIndex = start;
            pattern.endIndex = end;
            pattern.amplitude = absMean - minVal;
            pattern.periodMs = 0.0;
            pattern.confidence = confidence;
            pattern.description = tr("跌落: 索引%1~%2, 降至均值%.1f%%, 置信度%.0f%%")
                .arg(start).arg(end)
                .arg(minVal / absMean * kConfidenceScale)
                .arg(confidence * kConfidenceScale);

            m_patterns.append(pattern);
            ++m_stats.totalDropoutsFound;
            ++m_stats.totalPatternsDetected;

            if (confidence > m_stats.peakConfidence) {
                m_stats.peakConfidence = confidence;
            }

            emit patternDetected(pattern);
        } else {
            ++i;
        }
    }
}

// ============================================================================
// 周期模式检测 (自相关)
// ============================================================================

/** @brief 周期模式检测 -- 使用自相关函数识别重复信号模式
 *
 *  对缓冲区数据计算自相关函数，寻找除lag=0外的最大自相关峰值。
 *  若峰值超过0.5(强相关)，则认为存在周期模式。
 *  通过峰值位置估计周期，通过自相关强度计算置信度。
 */
void WaveformPatternDetector::detectPeriodicPatterns()
{
    int bufSize = m_buffer.size();
    if (bufSize < 2 || bufSize < m_config.minPatternLength * 2) return;

    /* 计算方差，常数信号无周期 */
    double stddev = (m_runningCount > 1)
        ? qSqrt(m_runningM2 / static_cast<double>(m_runningCount - 1))
        : 0.0;
    if (stddev < 1e-12) return;

    /* 搜索自相关峰值 -- 在[minPatternLength, bufSize/2]范围内 */
    int minLag = m_config.minPatternLength;
    int maxLag = bufSize / 2;

    double bestCorr = -2.0;
    int bestLag = 0;

    for (int lag = minLag; lag <= maxLag; ++lag) {
        double corr = autocorrelation(m_buffer, lag);
        if (corr > bestCorr) {
            bestCorr = corr;
            bestLag = lag;
        }
    }

    /* 自相关峰值阈值: 0.5表示强周期性 */
    const double kPeriodicThreshold = 0.5;
    if (bestCorr < kPeriodicThreshold || bestLag <= 0) return;

    /* 计算置信度: 自相关越强置信度越高 */
    double confidence = qBound(0.0, bestCorr, 1.0);

    /* 检测正弦波特征: 自相关在多个lag处呈现余弦衰减形态 */
    bool isSine = false;
    if (bestLag > 2) {
        /* 检查lag/2处是否也有较强自相关(半周期特征) */
        int halfLag = bestLag / 2;
        if (halfLag >= minLag) {
            double halfCorr = autocorrelation(m_buffer, halfLag);
            /* 正弦波在半周期处自相关为负值且绝对值较大 */
            if (halfCorr < -0.3) {
                isSine = true;
            }
        }
    }

    /* 计算信号峰峰值幅度 */
    double minVal = m_buffer[0];
    double maxVal = m_buffer[0];
    for (int k = 1; k < bufSize; ++k) {
        if (m_buffer[k] < minVal) minVal = m_buffer[k];
        if (m_buffer[k] > maxVal) maxVal = m_buffer[k];
    }

    DetectedPattern pattern;
    pattern.type = isSine ? PatternType::SineWave : PatternType::PeriodicPulse;
    pattern.startIndex = 0;
    pattern.endIndex = bufSize - 1;
    pattern.amplitude = maxVal - minVal;
    pattern.periodMs = static_cast<double>(bestLag); /* 采样点数作为周期单位 */
    pattern.confidence = confidence;
    pattern.description = isSine
        ? tr("正弦波: 周期%1点, 幅度%.3f, 自相关%.2f, 置信度%.0f%%")
            .arg(bestLag).arg(maxVal - minVal).arg(bestCorr).arg(confidence * kConfidenceScale)
        : tr("周期脉冲: 周期%1点, 幅度%.3f, 自相关%.2f, 置信度%.0f%%")
            .arg(bestLag).arg(maxVal - minVal).arg(bestCorr).arg(confidence * kConfidenceScale);

    m_patterns.append(pattern);
    ++m_stats.totalPeriodicFound;
    ++m_stats.totalPatternsDetected;

    if (confidence > m_stats.peakConfidence) {
        m_stats.peakConfidence = confidence;
    }

    emit patternDetected(pattern);
}

// ============================================================================
// 电平跳变检测
// ============================================================================

/** @brief 电平跳变检测 -- 识别信号均值的突然偏移
 *
 *  使用滑动窗口计算局部均值，比较相邻窗口的均值差异。
 *  差异超过2倍全局标准差时判定为电平跳变。
 */
void WaveformPatternDetector::detectLevelChanges()
{
    int bufSize = m_buffer.size();
    if (bufSize < 3 || bufSize < m_config.minPatternLength * 3) return;

    double stddev = (m_runningCount > 1)
        ? qSqrt(m_runningM2 / static_cast<double>(m_runningCount - 1))
        : 0.0;
    if (stddev < 1e-12) return;

    /* 使用半窗口大小的滑动均值 */
    int halfWindow = qMax(m_config.minPatternLength, bufSize / 8);
    const double kLevelChangeSigma = 2.0; ///< 电平跳变判定阈值(σ)

    /* 计算前半段和后半段的均值 */
    for (int pos = halfWindow; pos <= bufSize - halfWindow; pos += halfWindow / 2) {
        double meanBefore = 0.0;
        for (int j = pos - halfWindow; j < pos; ++j) {
            meanBefore += m_buffer[j];
        }
        meanBefore /= static_cast<double>(halfWindow);

        double meanAfter = 0.0;
        int afterLen = qMin(halfWindow, bufSize - pos);
        for (int j = pos; j < pos + afterLen; ++j) {
            meanAfter += m_buffer[j];
        }
        meanAfter /= static_cast<double>(afterLen);

        double shift = qAbs(meanAfter - meanBefore);
        if (shift > kLevelChangeSigma * stddev) {
            /* 置信度: 偏移越大置信度越高 */
            double confidence = qBound(0.0, shift / (kLevelChangeSigma * stddev) - 1.0, 1.0);

            /* 避免重复检测: 检查是否与已有电平跳变位置重叠 */
            bool duplicate = false;
            for (const auto& existing : m_patterns) {
                if (existing.type == PatternType::LevelChange
                    && qAbs(existing.startIndex - pos) < halfWindow) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) continue;

            DetectedPattern pattern;
            pattern.type = PatternType::LevelChange;
            pattern.startIndex = pos;
            pattern.endIndex = qMin(pos + afterLen - 1, bufSize - 1);
            pattern.amplitude = shift;
            pattern.periodMs = 0.0;
            pattern.confidence = confidence;
            pattern.description = tr("电平跳变: 位置%1, Δ=%.3f (%.1fσ), 置信度%.0f%%")
                .arg(pos).arg(shift).arg(shift / stddev).arg(confidence * kConfidenceScale);

            m_patterns.append(pattern);
            ++m_stats.totalLevelChanges;
            ++m_stats.totalPatternsDetected;

            if (confidence > m_stats.peakConfidence) {
                m_stats.peakConfidence = confidence;
            }

            emit patternDetected(pattern);
        }
    }
}

// ============================================================================
// 自相关计算
// ============================================================================

/** @brief 计算归一化自相关函数
 *  @param data 输入数据序列
 *  @param lag  滞后阶数
 *  @return lag处的归一化自相关值(-1.0 ~ 1.0)
 *
 *  公式: R(lag) = Σ(x[i]-mean)(x[i+lag]-mean) / Σ(x[i]-mean)^2
 *  当lag=0时返回1.0(完美自相关)。
 */
double WaveformPatternDetector::autocorrelation(const QVector<double>& data, int lag) const
{
    int n = data.size();
    if (n <= lag || lag <= 0) return 0.0;

    double mean = 0.0;
    for (int i = 0; i < n; ++i) {
        mean += data[i];
    }
    mean /= static_cast<double>(n);

    /* 分子: 交叉相关项 */
    double numerator = 0.0;
    for (int i = 0; i < n - lag; ++i) {
        numerator += (data[i] - mean) * (data[i + lag] - mean);
    }

    /* 分母: 自方差(用全部数据计算，保证归一化一致性) */
    double denominator = 0.0;
    for (int i = 0; i < n; ++i) {
        double diff = data[i] - mean;
        denominator += diff * diff;
    }

    if (denominator < 1e-20) return 0.0;

    return numerator / denominator;
}

// ============================================================================
// 查询接口
// ============================================================================

/** @brief 获取所有已检测到的模式 @return 模式列表 */
QList<WaveformPatternDetector::DetectedPattern> WaveformPatternDetector::detectedPatterns() const
{
    return m_patterns;
}

/** @brief 按类型筛选已检测到的模式 @param type 目标模式类型 @return 匹配的模式列表 */
QList<WaveformPatternDetector::DetectedPattern> WaveformPatternDetector::patternsByType(PatternType type) const
{
    QList<DetectedPattern> result;
    for (const auto& p : m_patterns) {
        if (p.type == type) {
            result.append(p);
        }
    }
    return result;
}

/** @brief 获取置信度最高的模式 @return 最佳模式(若无模式则返回空模式) */
WaveformPatternDetector::DetectedPattern WaveformPatternDetector::bestPattern() const
{
    if (m_patterns.isEmpty()) {
        return DetectedPattern{};
    }

    const DetectedPattern* best = &m_patterns[0];
    for (const auto& p : m_patterns) {
        if (p.confidence > best->confidence) {
            best = &p;
        }
    }
    return *best;
}

/** @brief 估计信号主频率(Hz) -- 从最佳周期模式推导
 *  @return 频率估计值(Hz)，无周期模式时返回0
 *
 *  注意: 此处返回的是归一化频率(1/周期)，若需转换为Hz，
 *  调用方需乘以实际采样率。
 */
double WaveformPatternDetector::estimatedFrequency() const
{
    /* 在所有周期模式中寻找置信度最高的 */
    double bestPeriod = 0.0;
    double bestConf = 0.0;

    for (const auto& p : m_patterns) {
        if ((p.type == PatternType::PeriodicPulse || p.type == PatternType::SineWave)
            && p.periodMs > 0.0 && p.confidence > bestConf) {
            bestPeriod = p.periodMs;
            bestConf = p.confidence;
        }
    }

    if (bestPeriod <= 0.0) return 0.0;

    return 1.0 / bestPeriod; /* 归一化频率 */
}

/** @brief 获取运行统计数据 @return Stats常量引用 */
const WaveformPatternDetector::Stats& WaveformPatternDetector::stats() const
{
    return m_stats;
}

/** @brief 清空采样缓冲区并重置在线统计状态 */
void WaveformPatternDetector::clearBuffer()
{
    m_buffer.clear();
    m_patterns.clear();
    m_sampleIndex = 0;
    m_runningMean = 0.0;
    m_runningM2 = 0.0;
    m_runningCount = 0;
}
