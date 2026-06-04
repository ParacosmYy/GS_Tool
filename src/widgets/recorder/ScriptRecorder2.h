/**
 * @file ScriptRecorder2.h
 * @brief 脚本录制回放组件 - 记录用户操作序列并支持定时回放
 *
 * 职责:
 *   1. 录制用户操作（发送命令、切换模式等）并附带时间戳
 *   2. 按录制时序通过定时器逐条回放操作
 *   3. 支持可调速回放（倍速控制）
 *   4. 提供录制条目的查询和清除操作
 */

#pragma once
#include <QWidget>
#include <QList>
#include <QString>
#include <QByteArray>
#include <QTimer>

/**
 * @brief 脚本录制回放组件
 *
 * 录制阶段通过 recordAction() 记录每条操作及其时间戳，
 * 回放阶段通过内部定时器按原始时间间隔逐条发射 playbackAction 信号。
 */
class ScriptRecorder : public QWidget {
    Q_OBJECT
public:
    /** @brief 录制条目结构体 */
    struct RecordEntry {
        qint64 timestamp;       ///< 相对录制起始的时间偏移（毫秒）
        QString action;         ///< 操作类型标识
        QByteArray data;        ///< 操作附带的数据
    };

    /**
     * @brief 构造脚本录制器
     * @param parent 父widget
     */
    explicit ScriptRecorder(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~ScriptRecorder() override;

    /** @brief 开始录制模式 */
    void startRecording();

    /** @brief 停止录制模式 */
    void stopRecording();

    /**
     * @brief 查询当前是否正在录制
     * @return true表示正在录制
     */
    bool isRecording() const;

    /**
     * @brief 记录一条操作（仅在录制模式下有效）
     * @param action 操作类型标识
     * @param data 操作附带的数据
     */
    void recordAction(const QString &action, const QByteArray &data);

    /** @brief 开始回放已录制的操作序列 */
    void playback();

    /** @brief 清空所有录制条目 */
    void clear();

    /**
     * @brief 获取所有录制条目
     * @return 录制条目列表
     */
    QList<RecordEntry> entries() const;

    /**
     * @brief 获取录制条目数量
     * @return 条目总数
     */
    int entryCount() const;

    /**
     * @brief 设置回放速度倍率
     * @param speed 速度倍率（1.0为原始速度，2.0为两倍速）
     */
    void setPlaybackSpeed(double speed);

signals:
    /** @brief 录制模式已启动 */
    void recordingStarted();

    /** @brief 录制模式已停止 @param entryCount 录制的条目总数 */
    void recordingStopped(int entryCount);

    /** @brief 一条操作已记录 @param action 操作类型标识 */
    void actionRecorded(const QString &action);

    /** @brief 回放已开始 */
    void playbackStarted();

    /** @brief 回放已完成 */
    void playbackFinished();

    /**
     * @brief 回放单条操作时发射
     * @param action 操作类型标识
     * @param data 操作附带的数据
     */
    void playbackAction(const QString &action, const QByteArray &data);

private:
    /** @brief 回放定时器回调：逐条发射playbackAction */
    void onPlaybackTick();

    QList<RecordEntry> m_entries;   ///< 录制条目列表
    bool m_recording = false;       ///< 是否正在录制
    bool m_playing = false;         ///< 是否正在回放
    double m_speed = 1.0;           ///< 回放速度倍率
    QTimer *m_timer = nullptr;      ///< 回放定时器
    int m_playIndex = 0;            ///< 当前回放位置索引

    // ---- 统计计数器 ----
    quint64 m_totalRecords = 0;      ///< 总录制条目数
    quint64 m_totalPlaybacks = 0;    ///< 总回放次数
    quint64 m_totalPlaybackActions = 0; ///< 总回放动作数

public:
    /** @brief 获取总录制条目数 @return 累计录制条目 */
    quint64 totalRecords() const { return m_totalRecords; }
    /** @brief 获取总回放次数 @return 累计回放 */
    quint64 totalPlaybacks() const { return m_totalPlaybacks; }
    /** @brief 获取总回放动作数 @return 累计回放动作 */
    quint64 totalPlaybackActions() const { return m_totalPlaybackActions; }
    /** @brief 重置录制器统计 */
    void resetRecorderStatistics() { m_totalRecords = 0; m_totalPlaybacks = 0; m_totalPlaybackActions = 0; }
};
