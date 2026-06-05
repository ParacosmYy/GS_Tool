/**
 * @file ProtocolReplayEngine.cpp
 * @brief 协议回放引擎实现
 *
 * 核心流程: loadScript → play → scheduleNextEntry (定时器) → emitCurrentEntry → 推进索引
 * 循环模式在到达结尾时重新开始; 乒乓模式在边界处反转方向。
 *
 * @author EmbedDebug Team
 * @version 1.0.0
 * @date 2026-06-05
 */

#include "protocol/replay/ProtocolReplayEngine.h"

// ─── 构造 / 析构 ────────────────────────────────────────────────

/**
 * @brief 构造函数 — 初始化定时器与信号连接
 * @param parent 父对象
 */
ProtocolReplayEngine::ProtocolReplayEngine(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("ProtocolReplayEngine"));

    m_playTimer.setSingleShot(true);
    connect(&m_playTimer, &QTimer::timeout,
            this, &ProtocolReplayEngine::emitCurrentEntry);
}

/**
 * @brief 析构函数 — 停止定时器，释放资源
 */
ProtocolReplayEngine::~ProtocolReplayEngine()
{
    stop();
}

// ─── 脚本管理 ────────────────────────────────────────────────────

/**
 * @brief 加载回放脚本
 *
 * 替换当前脚本，重置播放位置和循环计数。
 * 如果当前正在播放则先停止。
 *
 * @param script 待加载的脚本
 */
void ProtocolReplayEngine::loadScript(const ReplayScript& script)
{
    /* 先停止正在进行的回放 */
    if (m_playing) {
        stop();
    }

    m_script = script;
    m_currentIndex = 0;
    m_loopCount = 0;
    m_pingPongForward = true;

    /* 同步速度设置到脚本中 */
    ++m_stats.totalScriptsLoaded;
}

/**
 * @brief 清空当前脚本
 *
 * 停止回放并清空脚本内容。
 */
void ProtocolReplayEngine::clearScript()
{
    stop();
    m_script = ReplayScript{};
    m_currentIndex = 0;
    m_loopCount = 0;
    m_pingPongForward = true;
}

// ─── 播放控制 ────────────────────────────────────────────────────

/**
 * @brief 开始/继续播放
 *
 * 若脚本为空则发出错误信号并返回。
 * 若当前处于暂停状态，则恢复播放。
 * 否则从头开始播放，发送第一条目。
 */
void ProtocolReplayEngine::play()
{
    /* 空脚本检查 */
    if (m_script.entries.isEmpty()) {
        emit error(tr("无法播放: 脚本为空"));
        ++m_stats.playErrors;
        return;
    }

    /* 越界保护: 索引超出范围时复位 */
    if (m_currentIndex < 0 || m_currentIndex >= m_script.entries.size()) {
        m_currentIndex = 0;
    }

    /* 从暂停恢复 */
    if (m_paused) {
        m_paused = false;
        m_playing = true;
        m_entryTimer.start();
        emit resumed();
        scheduleNextEntry();
        return;
    }

    /* 全新播放 */
    m_playing = true;
    m_paused = false;
    ++m_stats.totalPlays;

    /* 立即发射第一条目 */
    m_entryTimer.start();
    emitCurrentEntry();
}

/**
 * @brief 暂停播放
 *
 * 停止定时器，保持当前位置。可通过 play() 恢复。
 */
void ProtocolReplayEngine::pause()
{
    if (!m_playing || m_paused) {
        return;
    }

    m_playTimer.stop();
    m_paused = true;
    m_playing = false;
    emit paused();
}

/**
 * @brief 停止播放并复位
 *
 * 停止定时器，将索引和循环计数归零。
 */
void ProtocolReplayEngine::stop()
{
    m_playTimer.stop();
    m_playing = false;
    m_paused = false;
    m_currentIndex = 0;
    m_loopCount = 0;
    m_pingPongForward = true;
}

/**
 * @brief 跳转到指定条目
 *
 * 仅在不播放时允许跳转。若索引越界则发出错误。
 *
 * @param index 目标条目索引
 */
void ProtocolReplayEngine::seekToEntry(int index)
{
    if (m_playing) {
        emit error(tr("播放期间不允许跳转，请先暂停"));
        ++m_stats.playErrors;
        return;
    }

    if (index < 0 || index >= m_script.entries.size()) {
        emit error(tr("跳转索引越界: %1 (有效范围 0~%2)")
                   .arg(index)
                   .arg(m_script.entries.size() - 1));
        ++m_stats.playErrors;
        return;
    }

    m_currentIndex = index;
}

// ─── 速度控制 ────────────────────────────────────────────────────

/**
 * @brief 设置回放速度
 * @param speed 速度枚举
 * @param customFactor 自定义因子 (仅 CustomSpeed 时使用，必须 > 0)
 */
void ProtocolReplayEngine::setSpeed(ReplaySpeed speed, double customFactor)
{
    m_script.speed = speed;

    if (speed == ReplaySpeed::CustomSpeed) {
        /* 自定义因子必须为正数 */
        m_script.customSpeedFactor = (customFactor > 0.0) ? customFactor : 1.0;
    } else {
        m_script.customSpeedFactor = 1.0;
    }
}

/**
 * @brief 获取当前速度设置
 * @return 速度枚举值
 */
ProtocolReplayEngine::ReplaySpeed ProtocolReplayEngine::speed() const
{
    return m_script.speed;
}

// ─── 状态查询 ────────────────────────────────────────────────────

/**
 * @brief 是否正在播放
 */
bool ProtocolReplayEngine::isPlaying() const
{
    return m_playing;
}

/**
 * @brief 是否处于暂停状态
 */
bool ProtocolReplayEngine::isPaused() const
{
    return m_paused;
}

/**
 * @brief 当前条目索引
 */
int ProtocolReplayEngine::currentEntryIndex() const
{
    return m_currentIndex;
}

/**
 * @brief 脚本中的条目总数
 */
int ProtocolReplayEngine::totalEntries() const
{
    return m_script.entries.size();
}

/**
 * @brief 获取当前脚本副本
 */
ProtocolReplayEngine::ReplayScript ProtocolReplayEngine::currentScript() const
{
    return m_script;
}

/**
 * @brief 获取统计信息
 */
ProtocolReplayEngine::Stats ProtocolReplayEngine::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计计数器
 */
void ProtocolReplayEngine::resetStatistics()
{
    m_stats = Stats{};
    m_sumEntryDelay = 0;
}

// ─── 内部调度 ────────────────────────────────────────────────────

/**
 * @brief 调度下一条目
 *
 * 计算下一条目的延迟时间并启动单次定时器。
 * 如果当前已到脚本末尾，根据回放模式决定后续行为:
 * - Single: 发射 scriptFinished 并停止
 * - Loop: 索引归零，递增循环计数，发射 scriptLooped
 * - PingPong: 反转方向，在边界处跳过重复条目
 */
void ProtocolReplayEngine::scheduleNextEntry()
{
    if (!m_playing) {
        return;
    }

    const int entryCount = m_script.entries.size();
    if (entryCount == 0) {
        m_playing = false;
        emit scriptFinished();
        return;
    }

    /* 计算下一个索引 */
    int nextIndex = m_currentIndex;

    switch (m_script.mode) {
    case ReplayMode::Single:
        nextIndex = m_currentIndex + 1;
        if (nextIndex >= entryCount) {
            /* 单次播放结束 */
            m_playing = false;
            emit scriptFinished();
            return;
        }
        break;

    case ReplayMode::Loop:
        nextIndex = m_currentIndex + 1;
        if (nextIndex >= entryCount) {
            nextIndex = 0;
            ++m_loopCount;
            ++m_stats.totalLoops;
            emit scriptLooped(m_loopCount);
        }
        break;

    case ReplayMode::PingPong:
        if (m_pingPongForward) {
            nextIndex = m_currentIndex + 1;
            if (nextIndex >= entryCount) {
                /* 到达正向末尾，反转方向 */
                m_pingPongForward = false;
                nextIndex = (entryCount > 1) ? (entryCount - 2) : 0;
                ++m_loopCount;
                ++m_stats.totalLoops;
                emit scriptLooped(m_loopCount);
            }
        } else {
            nextIndex = m_currentIndex - 1;
            if (nextIndex < 0) {
                /* 到达反向起点，恢复正向 */
                m_pingPongForward = true;
                nextIndex = (entryCount > 1) ? 1 : 0;
                ++m_loopCount;
                ++m_stats.totalLoops;
                emit scriptLooped(m_loopCount);
            }
        }
        break;
    }

    m_currentIndex = nextIndex;

    /* 计算延迟并启动定时器 */
    const qint64 delayMs = calculateDelay(m_script.entries.at(m_currentIndex));
    m_playTimer.start(static_cast<int>(delayMs));
}

/**
 * @brief 发射当前条目数据
 *
 * 根据 repeatCount 重复发送，更新统计信息，
 * 然后调度下一条目。
 */
void ProtocolReplayEngine::emitCurrentEntry()
{
    if (!m_playing || m_currentIndex < 0
        || m_currentIndex >= m_script.entries.size()) {
        return;
    }

    const ReplayEntry& entry = m_script.entries.at(m_currentIndex);

    /* 根据重复次数发送 (0次仍至少发1次以保持流程) */
    const int repeats = qMax(1, entry.repeatCount);
    for (int i = 0; i < repeats; ++i) {
        emit entryReady(entry.data, m_currentIndex);
    }

    /* 更新统计 */
    ++m_stats.totalEntriesPlayed;
    m_stats.totalBytesSent += static_cast<quint64>(entry.data.size() * repeats);

    /* 计算实际速度因子 */
    const qint64 elapsed = m_entryTimer.elapsed();
    if (elapsed > 0) {
        m_sumEntryDelay += static_cast<quint64>(elapsed);
        m_stats.actualSpeedFactor = calculateDelay(entry)
                                    / static_cast<double>(qMax(elapsed, qint64(1)));
    }

    /* 更新平均条目延迟 */
    if (m_stats.totalEntriesPlayed > 0) {
        m_stats.avgEntryDelayMs = static_cast<double>(m_sumEntryDelay)
                                  / static_cast<double>(m_stats.totalEntriesPlayed);
    }

    m_entryTimer.restart();

    /* 调度下一个条目 */
    scheduleNextEntry();
}

/**
 * @brief 计算条目的实际延迟时间
 *
 * 根据速度因子缩放条目的 relativeTimeMs。
 * 如果当前条目是脚本的第一条，延迟为 0 (立即发送)。
 * 两条目之间的延迟取相邻条目的 relativeTimeMs 差值。
 *
 * @param entry 目标条目
 * @return 实际延迟 (ms)，最小为 1ms
 */
qint64 ProtocolReplayEngine::calculateDelay(const ReplayEntry& entry) const
{
    /* 获取速度因子 */
    double factor = 1.0;
    switch (m_script.speed) {
    case ReplaySpeed::HalfSpeed:
        factor = 2.0;
        break;
    case ReplaySpeed::NormalSpeed:
        factor = 1.0;
        break;
    case ReplaySpeed::DoubleSpeed:
        factor = 0.5;
        break;
    case ReplaySpeed::CustomSpeed:
        factor = m_script.customSpeedFactor;
        break;
    }

    /*
     * 计算与下一条目的时间差。
     * 若当前是第一条目或只有一条目，使用条目自身的 relativeTimeMs 作为延迟。
     * 否则取差值。
     */
    qint64 baseDelay = entry.relativeTimeMs;

    if (m_currentIndex > 0) {
        const qint64 prevTime = m_script.entries.at(m_currentIndex - 1).relativeTimeMs;
        const qint64 diff = entry.relativeTimeMs - prevTime;
        /* 差值必须为正; 若脚本时序乱序则使用条目自身值 */
        baseDelay = (diff > 0) ? diff : entry.relativeTimeMs;
    }

    /* 应用速度因子并确保最小 1ms */
    qint64 adjustedDelay = static_cast<qint64>(baseDelay * factor);
    return qMax(adjustedDelay, qint64(1));
}
