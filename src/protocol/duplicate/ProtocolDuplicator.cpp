/**
 * @file ProtocolDuplicator.cpp
 * @brief 协议流量复制器实现 -- 录制/回放/修改/合并/比较/导入导出
 *
 * 核心流程:
 *   1. startRecording() 启动源连接的数据捕获,记录每帧的方向/数据/时间戳
 *   2. stopRecording() 停止捕获,保存为Recording对象存入录制库
 *   3. startDuplication() 按调度配置从Recording中逐条回放到目标连接
 *   4. 回放时通过ModificationRule对数据做字节级修改
 *   5. 支持多条录制的合并(按时序交错)和比较(逐字节差异)
 *
 * 统计重置方法见 ProtocolDuplicatorStats.cpp。
 */

#include "protocol/duplicate/ProtocolDuplicator.h"

#include <QJsonDocument>
#include <QFile>
#include <QDateTime>
#include <QUuid>
#include <algorithm>
#include <numeric>

// ============================================================================
// 构造 / 析构
// ============================================================================

/** @brief 构造协议流量复制器 @param parent 父对象 */
ProtocolDuplicator::ProtocolDuplicator(QObject* parent)
    : QObject(parent)
    , m_playbackTimer(new QTimer(this))
{
    setObjectName("ProtocolDuplicator");
    m_playbackTimer->setSingleShot(true);
    connect(m_playbackTimer, &QTimer::timeout,
            this, &ProtocolDuplicator::scheduleNextEntry);
}

/** @brief 析构(停止所有定时器和回放) */
ProtocolDuplicator::~ProtocolDuplicator()
{
    if (m_duplicating) {
        m_playbackTimer->stop();
        m_duplicating = false;
    }
}

// ============================================================================
// 连接绑定
// ============================================================================

/**
 * @brief 设置源连接
 * @param source 源连接指针(录制来源,可以为nullptr解除绑定)
 *
 * 如果当前正在录制,先停止录制。连接断开旧信号,绑定新信号。
 */
void ProtocolDuplicator::setSource(IConnection* source)
{
    if (m_recording) {
        stopRecording();
    }
    disconnectSourceSignals();
    m_source = source;
    if (m_source) {
        connectSourceSignals();
    }
}

/**
 * @brief 设置目标连接
 * @param target 目标连接指针(回放去向,可以为nullptr解除绑定)
 */
void ProtocolDuplicator::setTarget(IConnection* target)
{
    m_target = target;
}

/** @brief 获取当前源连接 @return 源连接指针,可能为nullptr */
IConnection* ProtocolDuplicator::source() const
{
    return m_source;
}

/** @brief 获取当前目标连接 @return 目标连接指针,可能为nullptr */
IConnection* ProtocolDuplicator::target() const
{
    return m_target;
}

// ============================================================================
// 录制控制
// ============================================================================

/**
 * @brief 开始录制
 * @param label 录制标签(空则自动生成)
 *
 * 流程:
 *   1. 校验源连接有效性
 *   2. 创建新的Recording对象,生成UUID和起始时间戳
 *   3. 启动QElapsedTimer用于计算帧间相对偏移
 *   4. 发射recordingStarted信号
 */
void ProtocolDuplicator::startRecording(const QString& label)
{
    if (m_recording) {
        emit error(tr("已在录制中,请先停止当前录制"));
        return;
    }
    if (!m_source) {
        emit error(tr("未设置源连接,无法录制"));
        return;
    }

    /* 初始化录制对象 */
    m_currentRecording = Recording();
    m_currentRecording.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_currentRecording.label = label.isEmpty()
        ? QString("Recording_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"))
        : label;
    m_currentRecording.startTimestampMs = QDateTime::currentMSecsSinceEpoch();
    m_currentRecording.sourceName = m_source ? m_source->name() : QString();

    m_recordTimer.start();
    m_recording = true;
    emit recordingStarted();
}

/**
 * @brief 停止录制并保存
 *
 * 流程:
 *   1. 标记录制结束时间
 *   2. 将录制存入录制库
 *   3. 更新统计
 *   4. 发射recordingStopped信号
 */
void ProtocolDuplicator::stopRecording()
{
    if (!m_recording) {
        return;
    }

    m_recording = false;
    m_currentRecording.endTimestampMs = QDateTime::currentMSecsSinceEpoch();

    /* 保存到录制库 */
    m_recordings.append(m_currentRecording);
    m_stats.totalRecorded += static_cast<quint64>(m_currentRecording.entries.size());

    /* 累计录制字节数 */
    quint64 bytes = 0;
    for (const TrafficEntry& e : m_currentRecording.entries) {
        bytes += static_cast<quint64>(e.data.size());
    }
    m_stats.totalBytesRecorded += bytes;

    emit recordingStopped();
}

/** @brief 是否正在录制 @return true=录制中 */
bool ProtocolDuplicator::isRecording() const
{
    return m_recording;
}

/** @brief 获取当前进行中的录制 @return Recording副本 */
ProtocolDuplicator::Recording ProtocolDuplicator::currentRecording() const
{
    return m_currentRecording;
}

// ============================================================================
// 回放控制
// ============================================================================

/**
 * @brief 开始回放
 * @param recording 要回放的录制
 * @param mode 调度模式(单次/重复N次/定时间隔/持续循环)
 * @param repeatCount 重复次数(mode=RepeatN时有效)
 * @param intervalMs 帧间间隔毫秒数(0=使用录制原始时序)
 *
 * 流程:
 *   1. 校验目标连接有效性和录制非空
 *   2. 按调度模式初始化回放状态
 *   3. 发射duplicationStarted信号
 *   4. 调度第一个条目
 */
void ProtocolDuplicator::startDuplication(const Recording& recording,
                                          ScheduleMode mode,
                                          int repeatCount,
                                          int intervalMs)
{
    if (m_duplicating) {
        emit error(tr("已在回放中,请先停止当前回放"));
        return;
    }
    if (!m_target) {
        emit error(tr("未设置目标连接,无法回放"));
        return;
    }
    if (recording.entries.isEmpty()) {
        emit error(tr("录制数据为空,无法回放"));
        return;
    }

    m_playbackRecording = recording;
    m_scheduleMode = mode;
    m_repeatCount = (repeatCount > 0) ? repeatCount : 1;
    m_intervalMs = qMax(0, intervalMs);
    m_currentRepeatIndex = 0;
    m_playbackEntryIndex = 0;
    m_duplicating = true;

    emit duplicationStarted();
    scheduleNextEntry();
}

/**
 * @brief 停止回放
 *
 * 停止回放定时器,重置回放状态,更新统计。
 */
void ProtocolDuplicator::stopDuplication()
{
    if (!m_duplicating) {
        return;
    }

    m_playbackTimer->stop();
    m_duplicating = false;
    m_playbackEntryIndex = 0;
    m_currentRepeatIndex = 0;
}

/** @brief 是否正在回放 @return true=回放中 */
bool ProtocolDuplicator::isDuplicating() const
{
    return m_duplicating;
}

/**
 * @brief 设置修改规则
 * @param rules 修改规则列表(回放时对数据做字节级替换)
 */
void ProtocolDuplicator::setModificationRules(const QList<ModificationRule>& rules)
{
    m_modificationRules = rules;
}

/** @brief 获取当前修改规则 @return 修改规则列表副本 */
QList<ProtocolDuplicator::ModificationRule> ProtocolDuplicator::modificationRules() const
{
    return m_modificationRules;
}

// ============================================================================
// 录制管理
// ============================================================================

/** @brief 获取所有已保存的录制 @return 录制列表副本 */
QList<ProtocolDuplicator::Recording> ProtocolDuplicator::recordings() const
{
    return m_recordings;
}

/**
 * @brief 添加一条录制到录制库
 * @param recording 要添加的录制
 */
void ProtocolDuplicator::addRecording(const Recording& recording)
{
    if (!recording.id.isEmpty()) {
        m_recordings.append(recording);
    }
}

/**
 * @brief 按ID删除录制
 * @param id 录制UUID
 */
void ProtocolDuplicator::removeRecording(const QString& id)
{
    m_recordings.removeIf([&id](const Recording& r) {
        return r.id == id;
    });
}

/** @brief 清空所有录制 */
void ProtocolDuplicator::clearRecordings()
{
    m_recordings.clear();
}

// ============================================================================
// 合并
// ============================================================================

/**
 * @brief 合并多条录制为一条
 * @param recordings 要合并的录制列表
 * @return 合并后的新录制(按时序交错排序)
 *
 * 算法: 收集所有条目,按时间戳排序,重新编号序列号。
 * 合并后的录制起始时间为最早条目的时间戳。
 */
ProtocolDuplicator::Recording ProtocolDuplicator::mergeRecordings(
    const QList<Recording>& recordings) const
{
    Recording merged;
    merged.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    merged.label = QString("Merged_%1").arg(
        QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));

    if (recordings.isEmpty()) {
        return merged;
    }

    /* 收集所有条目 */
    QList<TrafficEntry> allEntries;
    qint64 earliestTs = std::numeric_limits<qint64>::max();

    for (const Recording& rec : recordings) {
        for (const TrafficEntry& entry : rec.entries) {
            allEntries.append(entry);
            if (entry.timestampMs < earliestTs && entry.timestampMs > 0) {
                earliestTs = entry.timestampMs;
            }
        }
        if (rec.startTimestampMs < earliestTs && rec.startTimestampMs > 0) {
            earliestTs = rec.startTimestampMs;
        }
    }

    /* 按时间戳排序 */
    std::sort(allEntries.begin(), allEntries.end(),
              [](const TrafficEntry& a, const TrafficEntry& b) {
                  return a.timestampMs < b.timestampMs;
              });

    /* 重新编号并计算相对偏移 */
    merged.startTimestampMs = (earliestTs < std::numeric_limits<qint64>::max())
        ? earliestTs : QDateTime::currentMSecsSinceEpoch();
    merged.endTimestampMs = 0;

    for (int i = 0; i < allEntries.size(); ++i) {
        TrafficEntry entry = allEntries[i];
        entry.sequenceIndex = i;
        entry.relativeOffsetMs = entry.timestampMs - merged.startTimestampMs;
        if (entry.timestampMs > merged.endTimestampMs) {
            merged.endTimestampMs = entry.timestampMs;
        }
        merged.entries.append(entry);
    }

    return merged;
}

// ============================================================================
// 比较
// ============================================================================

/**
 * @brief 比较两条录制
 * @param a 录制A
 * @param b 录制B
 * @return ComparisonResult 比较结果
 *
 * 比较策略:
 *   1. 逐条目比较数据内容(按序列号对齐)
 *   2. 记录条目数差异、数据字节差异
 *   3. 生成人类可读的差异摘要
 */
ProtocolDuplicator::ComparisonResult ProtocolDuplicator::compareRecordings(
    const Recording& a, const Recording& b) const
{
    ComparisonResult result;
    result.identical = false;
    result.matchingEntries = 0;
    result.differentEntries = 0;
    result.onlyInA = 0;
    result.onlyInB = 0;

    qint64 maxCount = qMax(a.entries.size(), b.entries.size());
    qint64 minCount = qMin(a.entries.size(), b.entries.size());

    result.onlyInA = static_cast<qint64>(a.entries.size()) - minCount;
    result.onlyInB = static_cast<qint64>(b.entries.size()) - minCount;

    /* 逐条目比较 */
    for (int i = 0; i < minCount; ++i) {
        if (a.entries[i].data == b.entries[i].data &&
            a.entries[i].direction == b.entries[i].direction) {
            result.matchingEntries++;
        } else {
            result.differentEntries++;
            result.diffIndices.append(i);
        }
    }

    result.identical = (result.differentEntries == 0 &&
                        result.onlyInA == 0 &&
                        result.onlyInB == 0);

    /* 生成摘要 */
    if (result.identical) {
        result.summary = tr("两条录制完全相同,共 %1 条目").arg(minCount);
    } else {
        QStringList parts;
        if (result.differentEntries > 0) {
            parts << tr("%1 条目数据不同").arg(result.differentEntries);
        }
        if (result.onlyInA > 0) {
            parts << tr("A多出 %1 条目").arg(result.onlyInA);
        }
        if (result.onlyInB > 0) {
            parts << tr("B多出 %1 条目").arg(result.onlyInB);
        }
        result.summary = parts.join(", ");
    }

    return result;
}

// ============================================================================
// 导出 / 导入
// ============================================================================

// (JSON序列化/导出导入/统计/私有方法移至 ProtocolDuplicatorIO.cpp)
