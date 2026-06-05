/**
 * @file SerialHealthMonitor.cpp
 * @brief 串口健康监控器 -- 核心实现
 *
 * 包含错误分类统计、错误率滑动窗口计算、健康评分算法、
 * 突发错误检测、错误模式自相关分析、告警机制和健康报告生成。
 */

#include "serial/health/SerialHealthMonitor.h"

#include <QDateTime>
#include <QtMath>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 常量定义
// ═══════════════════════════════════════════════════════════

/** @brief 突发检测窗口大小(毫秒) -- 2秒内的错误聚集视为突发 */
static constexpr qint64 BURST_WINDOW_MS = 2000;

/** @brief 突发检测最小错误数 -- 窗口内达到此数量才视为突发 */
static constexpr int BURST_MIN_ERRORS = 3;

/** @brief 错误时间戳最大保留数 -- 限制内存使用，超过时裁剪最旧的记录 */
static constexpr int MAX_ERROR_TIMESTAMPS = 1000;

/** @brief 自相关分析的最小错误时间戳数 -- 少于此数量无法判断模式 */
static constexpr int MIN_PATTERN_SAMPLES = 8;

/** @brief 周期性判定阈值 -- 自相关系数超过此值判定为周期性错误 */
static constexpr double PERIODIC_THRESHOLD = 0.6;

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

/**
 * @brief 构造串口健康监控器
 *
 * 设置 objectName 以支持 QSS 样式，启动每秒采样定时器，
 * 启动运行时间计时器。
 * @param parent 父对象
 */
SerialHealthMonitor::SerialHealthMonitor(QObject* parent)
    : QObject(parent)
    , m_stats{}
{
    setObjectName(QStringLiteral("SerialHealthMonitor"));

    /* 初始化错误率滑动窗口为零 */
    std::fill(std::begin(m_errorWindow), std::end(m_errorWindow), 0);

    /* 启动每秒采样定时器 */
    connect(&m_sampleTimer, &QTimer::timeout,
            this, &SerialHealthMonitor::onSampleTimer);
    m_sampleTimer.start(1000);

    /* 启动运行时间计时器 */
    m_uptimeTimer.start();
}

/** @brief 析构 -- 停止采样定时器 */
SerialHealthMonitor::~SerialHealthMonitor()
{
    m_sampleTimer.stop();
}

// ═══════════════════════════════════════════════════════════
// 数据喂入
// ═══════════════════════════════════════════════════════════

/**
 * @brief 喂入 Qt 串口错误事件
 *
 * 将 QSerialPort::SerialPortError 映射到统计计数器:
 *   - ResourceError → connectionDrops (串口资源丢失/设备断开)
 *   - 其他非 NoError/TimeoutError → totalOverrunErrors (通用错误桶)
 * 同时记录错误时间戳用于突发检测和模式分析。
 *
 * @param error Qt串口错误类型
 */
void SerialHealthMonitor::feedError(QSerialPort::SerialPortError error)
{
    /* NoError 不应计入统计 */
    if (error == QSerialPort::NoError) {
        return;
    }

    /* 按错误类型分类累加 */
    switch (error) {
    case QSerialPort::ResourceError:
        /* 设备断开/资源丢失 → 记录断线并计入通用错误 */
        m_stats.connectionDrops++;
        break;
    case QSerialPort::TimeoutError:
        /* 超时不属于硬件信号错误，不计入 */
        return;
    default:
        /* WriteError/ReadError/OpenError 等归入溢出桶 */
        m_stats.totalOverrunErrors++;
        break;
    }

    /* 记录错误事件(更新总计数、时间戳、突发检测) */
    recordError();
}

/**
 * @brief 喂入底层硬件错误事件
 *
 * 由上层驱动/平台代码在检测到底层信号错误时调用，
 * 精确分类到帧错误/奇偶错误/溢出错误/中断条件。
 *
 * @param error 硬件错误类型枚举
 */
void SerialHealthMonitor::feedHardwareError(HardwareError error)
{
    switch (error) {
    case HardwareError::FramingError:
        m_stats.totalFramingErrors++;
        break;
    case HardwareError::ParityError:
        m_stats.totalParityErrors++;
        break;
    case HardwareError::OverrunError:
        m_stats.totalOverrunErrors++;
        break;
    case HardwareError::BreakCondition:
        m_stats.totalBreakConditions++;
        break;
    }

    /* 记录错误事件(更新总计数、时间戳、突发检测) */
    recordError();
}

/**
 * @brief 喂入接收到的数据量
 *
 * 更新累计接收字节数，用于计算每KB错误率。
 * @param bytes 本次接收的字节数(必须 >= 0)
 */
void SerialHealthMonitor::feedData(int bytes)
{
    if (bytes <= 0) {
        return;
    }
    m_stats.totalBytesReceived += static_cast<quint64>(bytes);
}

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

/**
 * @brief 设置告警触发的错误率阈值
 *
 * 当每秒错误率超过此阈值且告警已启用时发射 alertTriggered 信号。
 * @param errorRate 每秒错误率阈值(>0.0)
 */
void SerialHealthMonitor::setErrorThreshold(double errorRate)
{
    m_errorThreshold = qMax(0.0, errorRate);
    /* 阈值变更后重置告警状态，允许重新触发 */
    m_alertActive = false;
}

/**
 * @brief 启用或禁用健康告警
 *
 * 禁用告警不会停止统计计算，只阻止 alertTriggered 信号发射。
 * @param enabled true=启用告警, false=禁用告警
 */
void SerialHealthMonitor::setAlertEnabled(bool enabled)
{
    m_alertEnabled = enabled;
    if (!enabled) {
        m_alertActive = false;
    }
}

// ═══════════════════════════════════════════════════════════
// 统计查询
// ═══════════════════════════════════════════════════════════

/**
 * @brief 重置所有统计计数器和内部状态
 *
 * 清空所有错误计数、错误时间戳、滑动窗口、运行时间计时器。
 * 重置后健康评分恢复为100.0。
 */
void SerialHealthMonitor::resetStatistics()
{
    m_stats = SerialHealthStats{};
    m_errorTimestamps.clear();
    std::fill(std::begin(m_errorWindow), std::end(m_errorWindow), 0);
    m_errorWindowIdx = 0;
    m_currentSecondErrors = 0;
    m_lastHealthScore = 100.0;
    m_alertActive = false;
    m_uptimeTimer.restart();
}

/**
 * @brief 生成完整健康报告
 *
 * 以 QVariantMap 形式返回所有健康指标，供 UI 层或脚本引擎使用。
 * 键名说明:
 *   - framingErrors/parityErrors/overrunErrors/breakConditions: 各类错误计数
 *   - totalErrors: 总错误计数
 *   - totalBytes: 总接收字节数
 *   - errorRatePerSec: 每秒错误率(滑动窗口)
 *   - errorRatePerKB: 每KB错误率
 *   - uptimeSec: 连接运行时间(秒)
 *   - connectionDrops: 连接断开次数
 *   - healthScore: 综合健康评分(0-100)
 *   - errorBursts: 突发错误次数
 *   - errorPattern: 错误模式(Periodic/Random/Unknown)
 * @return 完整健康报告 QVariantMap
 */
QVariantMap SerialHealthMonitor::getHealthReport() const
{
    QVariantMap report;
    report[QStringLiteral("framingErrors")]    = QVariant::fromValue(m_stats.totalFramingErrors);
    report[QStringLiteral("parityErrors")]     = QVariant::fromValue(m_stats.totalParityErrors);
    report[QStringLiteral("overrunErrors")]    = QVariant::fromValue(m_stats.totalOverrunErrors);
    report[QStringLiteral("breakConditions")]  = QVariant::fromValue(m_stats.totalBreakConditions);
    report[QStringLiteral("totalErrors")]      = QVariant::fromValue(m_stats.totalErrors);
    report[QStringLiteral("totalBytes")]       = QVariant::fromValue(m_stats.totalBytesReceived);
    report[QStringLiteral("errorRatePerSec")]  = m_stats.errorRatePerSecond;
    report[QStringLiteral("errorRatePerKB")]   = m_stats.errorRatePerKB;
    report[QStringLiteral("uptimeSec")]        = QVariant::fromValue(m_stats.uptimeSeconds);
    report[QStringLiteral("connectionDrops")]  = QVariant::fromValue(m_stats.connectionDrops);
    report[QStringLiteral("healthScore")]      = m_stats.healthScore;
    report[QStringLiteral("errorBursts")]      = QVariant::fromValue(m_stats.errorBursts);
    report[QStringLiteral("errorPattern")]     = analyzeErrorPattern();
    return report;
}

// ═══════════════════════════════════════════════════════════
// 定时器回调
// ═══════════════════════════════════════════════════════════

/**
 * @brief 采样定时器回调 -- 每秒执行
 *
 * 依次执行:
 *   1. 将当前秒错误计数写入滑动窗口环形缓冲区
 *   2. 更新运行时间
 *   3. 更新错误率指标
 *   4. 计算新的健康评分
 *   5. 评分变化时发射 healthScoreChanged 信号
 *   6. 检查告警条件
 */
void SerialHealthMonitor::onSampleTimer()
{
    /* 步骤1: 将当前秒的错误计数写入环形缓冲区 */
    m_errorWindow[m_errorWindowIdx] = m_currentSecondErrors;
    m_errorWindowIdx = (m_errorWindowIdx + 1) % ERROR_WINDOW_SIZE;
    m_currentSecondErrors = 0;

    /* 步骤2: 更新运行时间 */
    m_stats.uptimeSeconds = m_uptimeTimer.elapsed() / 1000;

    /* 步骤3: 更新错误率 */
    updateErrorRates();

    /* 步骤4: 计算健康评分 */
    m_stats.healthScore = calculateHealthScore();

    /* 步骤5: 评分变化时发射信号 */
    if (!qFuzzyCompare(m_stats.healthScore, m_lastHealthScore)) {
        emit healthScoreChanged(m_stats.healthScore);
        m_lastHealthScore = m_stats.healthScore;
    }

    /* 步骤6: 检查告警 */
    checkAlert();
}

// ═══════════════════════════════════════════════════════════
// 内部计算
// ═══════════════════════════════════════════════════════════

/**
 * @brief 记录一次错误事件
 *
 * 递增总错误计数和当前秒错误计数，记录时间戳用于突发检测，
 * 裁剪过旧的时间戳以限制内存占用，然后触发即时突发检测。
 */
void SerialHealthMonitor::recordError()
{
    /* 更新总错误计数 */
    m_stats.totalErrors++;
    /* 当前秒错误计数(用于滑动窗口) */
    m_currentSecondErrors++;

    /* 记录错误时间戳(用于突发检测) */
    m_errorTimestamps.append(QDateTime::currentMSecsSinceEpoch());
    /* 裁剪过旧的时间戳，限制内存占用 */
    if (m_errorTimestamps.size() > MAX_ERROR_TIMESTAMPS) {
        m_errorTimestamps.erase(m_errorTimestamps.begin(),
                                m_errorTimestamps.begin()
                                    + (m_errorTimestamps.size() - MAX_ERROR_TIMESTAMPS));
    }

    /* 即时检测突发 */
    detectBurst();
}

/**
 * @brief 计算综合健康评分
 *
 * 评分算法(满分100):
 *   - 错误率评分(权重70%): 基于10秒滑动窗口平均错误率
 *     - 0错误/秒 → 100分
 *     - >=5错误/秒 → 0分
 *     - 线性插值
 *   - 稳定性评分(权重30%): 基于连接断开次数和运行时间比
 *     - 无断开且运行>60秒 → 100分
 *     - 频繁断开(每分钟>1次) → 0分
 *
 * @return 健康评分(0-100)
 */
double SerialHealthMonitor::calculateHealthScore() const
{
    /* ── 错误率评分(0-100, 权重70%) ── */
    const double ratePenalty = qMin(1.0, m_stats.errorRatePerSecond / 5.0);
    const double errorScore = (1.0 - ratePenalty) * 100.0;

    /* ── 稳定性评分(0-100, 权重30%) ── */
    double stabilityScore = 100.0;
    if (m_stats.connectionDrops > 0) {
        if (m_stats.uptimeSeconds > 0) {
            /* 每分钟断开次数 */
            const double dropsPerMin = static_cast<double>(m_stats.connectionDrops)
                                       / (static_cast<double>(m_stats.uptimeSeconds) / 60.0);
            const double dropPenalty = qMin(1.0, dropsPerMin);
            stabilityScore = (1.0 - dropPenalty) * 100.0;
        } else {
            /* 运行时间为0但有断开记录: 连接极不稳定 */
            stabilityScore = 0.0;
        }
    }

    return qMax(0.0, qMin(100.0, errorScore * 0.7 + stabilityScore * 0.3));
}

/**
 * @brief 更新错误率指标
 *
 * 基于滑动窗口计算:
 *   - errorRatePerSecond: 最近10秒平均错误率
 *   - errorRatePerKB: 总错误数 / 总KB(总字节数/1024)
 */
void SerialHealthMonitor::updateErrorRates()
{
    /* 滑动窗口内总错误数 */
    int windowSum = 0;
    for (int i = 0; i < ERROR_WINDOW_SIZE; ++i) {
        windowSum += m_errorWindow[i];
    }
    m_stats.errorRatePerSecond = static_cast<double>(windowSum)
                                 / static_cast<double>(ERROR_WINDOW_SIZE);

    /* 每KB错误率 */
    if (m_stats.totalBytesReceived > 0) {
        const double totalKB = static_cast<double>(m_stats.totalBytesReceived) / 1024.0;
        m_stats.errorRatePerKB = (totalKB > 0.0)
            ? static_cast<double>(m_stats.totalErrors) / totalKB
            : 0.0;
    } else {
        m_stats.errorRatePerKB = 0.0;
    }
}

/**
 * @brief 检测错误突发
 *
 * 突发定义: 在 BURST_WINDOW_MS(2秒) 时间窗口内出现 >= BURST_MIN_ERRORS(3) 个错误。
 * 检测方法: 检查最近 BURST_MIN_ERRORS 个错误的时间跨度是否在窗口内。
 * 检测到突发时发射 errorBurstDetected 信号并递增累计突发计数。
 */
void SerialHealthMonitor::detectBurst()
{
    const int count = m_errorTimestamps.size();
    if (count < BURST_MIN_ERRORS) {
        return;
    }

    /* 检查最近 BURST_MIN_ERRORS 个错误的时间跨度 */
    const qint64 newest = m_errorTimestamps[count - 1];
    const qint64 oldest = m_errorTimestamps[count - BURST_MIN_ERRORS];
    const qint64 span = newest - oldest;

    if (span <= BURST_WINDOW_MS && span >= 0) {
        /* 去重: 仅当此突发与上次记录的突发不重叠时才计为新突发 */
        if (m_lastBurstTimestamp < oldest) {
            m_lastBurstTimestamp = newest;
            m_stats.errorBursts++;
            emit errorBurstDetected(BURST_MIN_ERRORS);
        }
    }
}

/**
 * @brief 分析错误模式
 *
 * 通过计算错误时间间隔序列的自相关系数(滞后1)判断错误模式:
 *   - 自相关系数 >= PERIODIC_THRESHOLD(0.6) → Periodic (周期性错误)
 *   - 自相关系数 < PERIODIC_THRESHOLD → Random (随机错误)
 *   - 错误时间戳不足 MIN_PATTERN_SAMPLES(8) → Unknown
 *
 * @return 错误模式字符串: "Periodic" / "Random" / "Unknown"
 */
QString SerialHealthMonitor::analyzeErrorPattern() const
{
    if (m_errorTimestamps.size() < MIN_PATTERN_SAMPLES) {
        return QStringLiteral("Unknown");
    }

    /* 计算错误时间间隔序列 */
    const int n = m_errorTimestamps.size();
    QVector<double> intervals;
    intervals.reserve(n - 1);
    for (int i = 1; i < n; ++i) {
        intervals.append(static_cast<double>(m_errorTimestamps[i] - m_errorTimestamps[i - 1]));
    }

    const int m = intervals.size();
    if (m < 2) {
        return QStringLiteral("Unknown");
    }

    /* 计算均值 */
    double mean = 0.0;
    for (double v : intervals) {
        mean += v;
    }
    mean /= static_cast<double>(m);

    /* 计算方差(滞后0自相关) */
    double variance = 0.0;
    for (double v : intervals) {
        variance += (v - mean) * (v - mean);
    }
    if (variance < 1e-10) {
        /* 所有间隔完全相同 → 高度周期性 */
        return QStringLiteral("Periodic");
    }

    /* 计算滞后1自相关系数: r(1) = sum((x[i]-mean)*(x[i+1]-mean)) / variance */
    double autocorr = 0.0;
    for (int i = 0; i < m - 1; ++i) {
        autocorr += (intervals[i] - mean) * (intervals[i + 1] - mean);
    }
    autocorr /= variance;

    return (autocorr >= PERIODIC_THRESHOLD)
        ? QStringLiteral("Periodic")
        : QStringLiteral("Random");
}

/**
 * @brief 检查告警条件
 *
 * 当以下条件同时满足时发射 alertTriggered 信号:
 *   - 告警已启用(m_alertEnabled)
 *   - 当前错误率超过阈值(m_errorThreshold)
 *   - 尚未处于告警状态(避免重复触发)
 *
 * 错误率低于阈值时自动解除告警状态。
 */
void SerialHealthMonitor::checkAlert()
{
    if (!m_alertEnabled) {
        return;
    }

    if (m_stats.errorRatePerSecond > m_errorThreshold) {
        if (!m_alertActive) {
            m_alertActive = true;
            const QString msg = tr("串口信号质量下降: 错误率 %1/s 超过阈值 %2/s, 健康评分 %3")
                                .arg(QString::number(m_stats.errorRatePerSecond, 'f', 2),
                                     QString::number(m_errorThreshold, 'f', 2),
                                     QString::number(m_stats.healthScore, 'f', 1));
            emit alertTriggered(msg);
        }
    } else {
        /* 错误率恢复到阈值以下，解除告警状态 */
        m_alertActive = false;
    }
}
