/**
 * @file ProtocolMonitor.cpp
 * @brief 协议监控器实现 -- 实时序列追踪、异常检测与时序分析
 *
 * 实现 ProtocolMonitor 的序列追踪、异常检测、时序统计和告警功能。
 * 监控器以状态机方式工作: 在预定义的序列步骤上逐条校验消息，
 * 发现偏差时记录异常并通过信号通知外部。
 */

#include "protocol/monitor/ProtocolMonitor.h"

#include <QDateTime>
#include <QStringBuilder>

// --------------------------------------------------------------------------
// 构造 / 析构
// --------------------------------------------------------------------------

/**
 * @brief 构造函数，初始化定时器和计时器
 * @param parent 父QObject(纳入QObject父子树)
 */
ProtocolMonitor::ProtocolMonitor(QObject* parent)
    : QObject(parent)
    , m_stepTimer(new QTimer(this))
{
    setObjectName(QStringLiteral("ProtocolMonitor"));

    //-- 超时定时器: 单次触发，检查当前步骤是否超时 --//
    m_stepTimer->setSingleShot(true);
    connect(m_stepTimer, &QTimer::timeout,
            this, [this]() { checkTimeoutForStep(); });
}

/**
 * @brief 析构函数，停止监控并清理资源
 */
ProtocolMonitor::~ProtocolMonitor()
{
    stopMonitoring();
}

// --------------------------------------------------------------------------
// 序列定义
// --------------------------------------------------------------------------

/** @brief 定义期望的消息序列，自动停止当前监控并重置 */
void ProtocolMonitor::defineSequence(const QList<StepDef>& steps)
{
    stopMonitoring();
    m_steps = steps;
    m_currentStep = -1;
}

/** @brief 获取当前序列定义 */
QList<ProtocolMonitor::StepDef> ProtocolMonitor::sequenceDefinition() const
{
    return m_steps;
}

// --------------------------------------------------------------------------
// 监控生命周期
// --------------------------------------------------------------------------

/** @brief 开始监控，重置序列位置到起点并启动计时器 */
void ProtocolMonitor::startMonitoring()
{
    if (m_steps.isEmpty()) {
        return;
    }

    m_monitoring = true;
    m_elapsed.start();
    m_lastMessageTimeMs = 0;
    resetSequencePosition();

    emit monitoringStateChanged(true);
}

/** @brief 停止监控，停止定时器，不清空统计 */
void ProtocolMonitor::stopMonitoring()
{
    if (!m_monitoring) {
        return;
    }

    m_monitoring = false;
    m_stepTimer->stop();
    m_currentStep = -1;

    emit monitoringStateChanged(false);
}

/** @brief 查询当前是否正在监控 */
bool ProtocolMonitor::isMonitoring() const
{
    return m_monitoring;
}

// --------------------------------------------------------------------------
// 数据输入
// --------------------------------------------------------------------------

/**
 * @brief 喂入一条协议消息进行校验
 * @param type 消息类型标识(与StepDef::type匹配)
 * @param data 消息负载数据(用于尺寸校验)
 *
 * 核心处理流程:
 *   1. 记录时间戳，计算消息间隔
 *   2. 更新消息计数和速率统计
 *   3. 如果当前处于序列追踪，校验消息类型和尺寸
 *   4. 匹配成功则推进步骤；失败则记录异常
 *   5. 检查异常率是否超过阈值
 */
void ProtocolMonitor::feedMessage(const QByteArray& type, const QByteArray& data)
{
    if (!m_monitoring) {
        return;
    }

    //-- 1. 时间戳和消息间隔 --//
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    if (m_lastMessageTimeMs > 0) {
        const double delayMs = static_cast<double>(nowMs - m_lastMessageTimeMs);
        m_sumInterMessageDelayMs += delayMs;
        ++m_interMessageCount;
    }
    m_lastMessageTimeMs = nowMs;

    //-- 2. 消息计数 --//
    ++m_stats.totalMessages;

    //-- 3. 序列步骤校验 --//
    if (m_currentStep >= 0 && m_currentStep < m_steps.size()) {
        const StepDef& expected = m_steps[m_currentStep];
        bool matched = false;

        //-- 3a. 检查消息类型是否匹配 --//
        if (type == expected.type) {
            matched = true;
        } else {
            //-- 如果不匹配，检查是否当前步骤可选且下一步匹配 --//
            int probeStep = m_currentStep;
            while (probeStep < m_steps.size() && m_steps[probeStep].optional) {
                ++probeStep;
                if (probeStep < m_steps.size() && type == m_steps[probeStep].type) {
                    //-- 跳过了可选步骤，直接匹配到后续步骤 --//
                    m_currentStep = probeStep;
                    matched = true;
                    break;
                }
            }
        }

        if (!matched) {
            //-- 3b. 类型不匹配: 记录意外消息异常 --//
            const QString desc = QStringLiteral("意外消息类型: 期望 '%1', 实际 '%2'")
                                     .arg(QString::fromUtf8(expected.type),
                                          QString::fromUtf8(type));
            emitAnomaly(UnexpectedMessage, desc, type);

            //-- 严格模式: 直接重置序列 --//
            if (m_strictMode) {
                const QString resetDesc = QStringLiteral("严格模式下因意外消息重置序列");
                emitAnomaly(SequenceDeviation, resetDesc, type);
                resetSequencePosition();
                emit sequenceReset();
                updateStats();
                return;
            }
            updateStats();
            return;
        }

        //-- 3c. 类型匹配，检查尺寸 --//
        const int dataSize = data.size();
        if (dataSize < expected.minSize || dataSize > expected.maxSize) {
            const QString desc = QStringLiteral("消息尺寸越界: 步骤[%0] '%1' "
                                                "期望 [%2, %3], 实际 %4 字节")
                                     .arg(m_currentStep)
                                     .arg(QString::fromUtf8(expected.type))
                                     .arg(expected.minSize)
                                     .arg(expected.maxSize)
                                     .arg(dataSize);
            emitAnomaly(SizeViolation, desc, type);
            ++m_stats.totalSizeViolations;
            //-- 尺寸异常不阻止步骤推进 --//
        }

        //-- 3d. 停止当前步骤超时定时器，推进 --//
        m_stepTimer->stop();
        advanceToNextStep();
    }

    //-- 4. 更新派生统计 --//
    updateStats();

    //-- 5. 检查异常率阈值 --//
    if (m_stats.totalMessages > 10 && m_stats.anomalyRate > m_anomalyThreshold) {
        const QString desc = QStringLiteral("异常率 %.1% 超过阈值 %.1%")
                                 .arg(m_stats.anomalyRate * 100.0, 0, 'f', 1)
                                 .arg(m_anomalyThreshold * 100.0, 0, 'f', 1);
        emitAnomaly(ExcessiveAnomalyRate, desc, type);
    }
}

// --------------------------------------------------------------------------
// 配置
// --------------------------------------------------------------------------

/** @brief 设置异常率告警阈值(0.0~1.0) */
void ProtocolMonitor::setAnomalyThreshold(double rate)
{
    m_anomalyThreshold = qBound(0.0, rate, 1.0);
}

/** @brief 获取异常率告警阈值 */
double ProtocolMonitor::anomalyThreshold() const
{
    return m_anomalyThreshold;
}

/**
 * @brief 设置序列完成后是否自动循环
 * @param enabled true=完成后从第一步重新开始
 */
void ProtocolMonitor::setLoopEnabled(bool enabled)
{
    m_loopEnabled = enabled;
}

/**
 * @brief 获取循环模式状态
 */
bool ProtocolMonitor::isLoopEnabled() const
{
    return m_loopEnabled;
}

/**
 * @brief 设置严格模式
 * @param enabled true=意外消息直接重置序列；false=仅记录异常继续当前步骤
 */
void ProtocolMonitor::setStrictMode(bool enabled)
{
    m_strictMode = enabled;
}

/**
 * @brief 获取严格模式状态
 */
bool ProtocolMonitor::isStrictMode() const
{
    return m_strictMode;
}

// --------------------------------------------------------------------------
// 统计
// --------------------------------------------------------------------------

/**
 * @brief 获取运行统计(只读引用)
 */
const ProtocolMonitor::MonitorStats& ProtocolMonitor::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计计数器和辅助变量
 *
 * 不影响序列定义、监控状态和配置。
 */
void ProtocolMonitor::resetStatistics()
{
    m_stats = MonitorStats{};
    m_sumInterMessageDelayMs = 0.0;
    m_interMessageCount = 0;
    m_lastMessageTimeMs = 0;
}

// --------------------------------------------------------------------------
// 异常历史
// --------------------------------------------------------------------------

/**
 * @brief 获取异常记录列表
 */
QList<ProtocolMonitor::AnomalyRecord> ProtocolMonitor::anomalyHistory() const
{
    return m_anomalyHistory;
}

/**
 * @brief 清空异常历史
 */
void ProtocolMonitor::clearAnomalyHistory()
{
    m_anomalyHistory.clear();
}

// --------------------------------------------------------------------------
// 序列进度
// --------------------------------------------------------------------------

/**
 * @brief 获取当前步骤索引
 * @return 步骤索引(0-based)，-1表示未开始或未监控
 */
int ProtocolMonitor::currentStepIndex() const
{
    return m_currentStep;
}

/**
 * @brief 获取序列总步骤数
 */
int ProtocolMonitor::totalSteps() const
{
    return m_steps.size();
}

// --------------------------------------------------------------------------
// 私有方法
// --------------------------------------------------------------------------

/**
 * @brief 推进到下一个序列步骤
 *
 * 如果已到最后一步，发射 sequenceComplete() 信号。
 * 若循环模式开启则重新开始；否则停止监控。
 */
void ProtocolMonitor::advanceToNextStep()
{
    ++m_currentStep;

    if (m_currentStep >= m_steps.size()) {
        //-- 序列走完 --//
        ++m_stats.completedSequences;
        emit sequenceComplete();

        if (m_loopEnabled) {
            //-- 循环模式: 从头开始 --//
            m_currentStep = 0;
            startStepTimer();
            emit sequenceReset();
        } else {
            //-- 非循环: 停在末尾，停止监控 --//
            m_currentStep = -1;
            stopMonitoring();
        }
        return;
    }

    //-- 为新步骤启动超时定时器 --//
    startStepTimer();
}

/**
 * @brief 重置序列位置到起点并启动第一步的超时定时器
 */
void ProtocolMonitor::resetSequencePosition()
{
    m_currentStep = m_steps.isEmpty() ? -1 : 0;
    if (m_currentStep >= 0) {
        startStepTimer();
    }
}

/**
 * @brief 检查当前步骤是否超时
 *
 * 由 m_stepTimer 单次超时触发。可选步骤超时不计为异常。
 */
void ProtocolMonitor::checkTimeoutForStep()
{
    if (!m_monitoring || m_currentStep < 0 || m_currentStep >= m_steps.size()) {
        return;
    }

    const StepDef& step = m_steps[m_currentStep];

    if (step.optional) {
        //-- 可选步骤超时: 自动跳过，尝试下一步 --//
        advanceToNextStep();
        return;
    }

    const QString desc = QStringLiteral("步骤[%0] '%1' 等待超时 (%2ms)")
                             .arg(m_currentStep)
                             .arg(QString::fromUtf8(step.type))
                             .arg(step.timeoutMs);
    emitAnomaly(Timeout, desc, QByteArray());
    ++m_stats.totalTimeouts;

    //-- 超时后推进到下一步 --//
    advanceToNextStep();
}

/**
 * @brief 发射异常信号并记录到历史
 * @param type 异常类型
 * @param description 异常描述
 * @param actualType 实际消息类型(可能为空)
 */
void ProtocolMonitor::emitAnomaly(AnomalyType type,
                                  const QString& description,
                                  const QByteArray& actualType)
{
    //-- 构造异常记录 --//
    AnomalyRecord record;
    record.type = type;
    record.description = description;
    record.actualType = actualType;
    record.expectedStepIndex = m_currentStep;
    record.timestampMs = QDateTime::currentMSecsSinceEpoch();

    //-- 限制历史大小 --//
    if (m_anomalyHistory.size() >= kMaxAnomalyHistory) {
        m_anomalyHistory.removeFirst();
    }
    m_anomalyHistory.append(record);

    //-- 更新计数 --//
    ++m_stats.totalAnomalies;
    if (type == SequenceDeviation) {
        ++m_stats.totalSequenceDeviations;
    }

    //-- 发射信号: 异常类型名称 + 描述 --//
    const QString typeName = QMetaEnum::fromType<AnomalyType>().valueToKey(type);
    emit anomalyDetected(typeName, description);
}

/**
 * @brief 重新计算派生统计值
 *
 * 计算: 平均消息间隔、消息速率、异常率。
 * 在每次 feedMessage 后调用以保持统计实时性。
 */
void ProtocolMonitor::updateStats()
{
    //-- 平均消息间隔 --//
    if (m_interMessageCount > 0) {
        m_stats.avgInterMessageDelayMs =
            m_sumInterMessageDelayMs / static_cast<double>(m_interMessageCount);
    }

    //-- 消息速率: 利用elapsed计时器计算 --//
    const qint64 elapsedMs = m_elapsed.elapsed();
    if (elapsedMs > 0) {
        m_stats.messageRate =
            static_cast<double>(m_stats.totalMessages) / (static_cast<double>(elapsedMs) / 1000.0);
    }

    //-- 异常率 --//
    if (m_stats.totalMessages > 0) {
        m_stats.anomalyRate =
            static_cast<double>(m_stats.totalAnomalies) / static_cast<double>(m_stats.totalMessages);
    } else {
        m_stats.anomalyRate = 0.0;
    }
}

/**
 * @brief 为当前步骤启动超时定时器
 *
 * 定时器为单次触发，超时后调用 checkTimeoutForStep()。
 */
void ProtocolMonitor::startStepTimer()
{
    if (m_currentStep < 0 || m_currentStep >= m_steps.size()) {
        return;
    }

    const int timeoutMs = m_steps[m_currentStep].timeoutMs;
    if (timeoutMs > 0) {
        m_stepTimer->start(timeoutMs);
    }
}
