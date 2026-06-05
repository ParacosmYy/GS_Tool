/**
 * @file ProtocolReplayEngine.h
 * @brief 协议回放引擎 — 录制、编辑、回放协议报文序列
 *
 * 支持可配置的时序控制、变速回放、循环/乒乓模式。
 * 通过 entryReady 信号将待发送数据交给外部连接层，
 * 引擎本身不持有任何连接对象。
 *
 * @author EmbedDebug Team
 * @version 1.0.0
 * @date 2026-06-05
 */

#ifndef PROTOCOL_REPLAY_ENGINE_H
#define PROTOCOL_REPLAY_ENGINE_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QTimer>
#include <QElapsedTimer>
#include <QString>

/**
 * @class ProtocolReplayEngine
 * @brief 协议报文序列回放引擎
 *
 * 职责:
 * - 加载 ReplayScript 脚本并按序回放
 * - 支持 Single / Loop / PingPong 三种回放模式
 * - 支持 HalfSpeed / NormalSpeed / DoubleSpeed / CustomSpeed 变速
 * - 通过 entryReady 信号通知外部发送数据
 * - 内置统计计数器 (总条目数、字节数、循环数等)
 */
class ProtocolReplayEngine : public QObject
{
    Q_OBJECT

public:
    /** @brief 回放速度枚举 */
    enum class ReplaySpeed {
        HalfSpeed,      ///< 半速 — 延迟 x2
        NormalSpeed,    ///< 正常速度
        DoubleSpeed,    ///< 双倍速 — 延迟 x0.5
        CustomSpeed     ///< 自定义倍速
    };
    Q_ENUM(ReplaySpeed)

    /** @brief 回放模式枚举 */
    enum class ReplayMode {
        Single,     ///< 单次播放到结尾后停止
        Loop,       ///< 循环播放，到结尾后重新开始
        PingPong    ///< 乒乓模式，正向播放到结尾后反向回播
    };
    Q_ENUM(ReplayMode)

    /** @brief 单条回放条目 */
    struct ReplayEntry {
        QByteArray data;            ///< 待发送的原始数据
        QString label;              ///< 可读标签 (用于UI展示)
        qint64 relativeTimeMs = 0;  ///< 相对于脚本起始的时间偏移 (ms)
        int repeatCount = 1;        ///< 该条目连续发送次数
    };

    /** @brief 回放脚本 — 由多个 ReplayEntry 组成 */
    struct ReplayScript {
        QList<ReplayEntry> entries;         ///< 条目列表
        QString name;                       ///< 脚本名称
        ReplayMode mode = ReplayMode::Single;       ///< 回放模式
        ReplaySpeed speed = ReplaySpeed::NormalSpeed;///< 回放速度
        double customSpeedFactor = 1.0;     ///< 自定义速度因子 (仅 CustomSpeed 时生效)
    };

    /** @brief 运行时统计信息 */
    struct Stats {
        quint64 totalEntriesPlayed = 0;     ///< 已播放条目总数
        quint64 totalBytesSent = 0;         ///< 已发送字节总数
        quint64 totalScriptsLoaded = 0;     ///< 已加载脚本总数
        quint64 totalPlays = 0;             ///< 总播放次数 (含循环)
        quint64 totalLoops = 0;             ///< 循环次数
        quint64 playErrors = 0;             ///< 播放出错次数
        double avgEntryDelayMs = 0.0;       ///< 平均条目间延迟 (ms)
        double actualSpeedFactor = 1.0;     ///< 实际速度因子
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ProtocolReplayEngine(QObject* parent = nullptr);

    /** @brief 析构函数 — 自动停止回放 */
    ~ProtocolReplayEngine() override;

    /** @brief 加载回放脚本，重置播放位置 */
    void loadScript(const ReplayScript& script);

    /** @brief 清空当前脚本并重置状态 */
    void clearScript();

    /** @brief 从当前位置开始/继续播放 */
    void play();

    /** @brief 暂停播放 (可 resume) */
    void pause();

    /** @brief 停止播放并复位到起始位置 */
    void stop();

    /**
     * @brief 跳转到指定条目索引
     * @param index 目标索引 (必须在有效范围内)
     */
    void seekToEntry(int index);

    /**
     * @brief 设置回放速度
     * @param speed 速度枚举
     * @param customFactor 自定义因子 (仅 CustomSpeed 时使用)
     */
    void setSpeed(ReplaySpeed speed, double customFactor = 1.0);

    /** @brief 获取当前速度设置 */
    ReplaySpeed speed() const;

    /** @brief 是否正在播放 */
    bool isPlaying() const;

    /** @brief 是否处于暂停状态 */
    bool isPaused() const;

    /** @brief 当前条目索引 */
    int currentEntryIndex() const;

    /** @brief 脚本中的条目总数 */
    int totalEntries() const;

    /** @brief 获取当前加载的脚本副本 */
    ReplayScript currentScript() const;

    /** @brief 获取统计信息 */
    Stats stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 一条数据已准备就绪，可发送 */
    void entryReady(const QByteArray& data, int index);

    /** @brief 脚本播放完成 (Single 模式) */
    void scriptFinished();

    /** @brief 脚本完成一轮循环 */
    void scriptLooped(int loopCount);

    /** @brief 播放已暂停 */
    void paused();

    /** @brief 播放已恢复 */
    void resumed();

    /** @brief 发生错误 */
    void error(const QString& message);

private:
    ReplayScript m_script;          ///< 当前脚本
    QTimer m_playTimer;             ///< 单次触发定时器，控制条目间隔
    QElapsedTimer m_entryTimer;     ///< 测量上一条目的实际耗时
    bool m_playing = false;         ///< 正在播放标志
    bool m_paused = false;          ///< 已暂停标志
    int m_currentIndex = 0;         ///< 当前条目索引
    int m_loopCount = 0;            ///< 累计循环次数
    bool m_pingPongForward = true;  ///< 乒乓模式方向: true=正向
    Stats m_stats;                  ///< 统计数据
    quint64 m_sumEntryDelay = 0;    ///< 条目延迟累计 (用于计算平均值)

    /** @brief 调度下一条目: 计算延迟并启动单次定时器 */
    void scheduleNextEntry();

    /** @brief 发射当前条目数据并推进索引 */
    void emitCurrentEntry();

    /**
     * @brief 根据速度因子计算条目间延迟
     * @param entry 目标条目
     * @return 实际延迟 (ms)
     */
    qint64 calculateDelay(const ReplayEntry& entry) const;
};

#endif // PROTOCOL_REPLAY_ENGINE_H
