/**
 * @file RoundRobinScheduler.h
 * @brief 轮询调度器(Round Robin Scheduler)
 */

#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>
#include <QQueue>

/**
 * @class RoundRobinScheduler
 * @brief 轮询调度器 — 公平分配时间片的任务调度
 *
 * 支持权重化的轮询调度、优先级插队和任务统计。
 * 适用于负载均衡、任务队列、网络调度等场景。
 */
class RoundRobinScheduler : public QObject
{
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalScheduled = 0;    /**< 总调度次数 */
        int totalTasksAdded = 0;   /**< 总添加任务数 */
        int totalTasksRemoved = 0; /**< 总移除任务数 */
        double avgProcessingTimeMs = 0.0; /**< 平均处理时间(ms) */
    };

    /** @brief 构造函数 */
    explicit RoundRobinScheduler(QObject* parent = nullptr);

    /**
     * @brief 添加任务
     * @param taskId 任务标识
     * @param weight 权重(默认1)
     */
    void addTask(const QString& taskId, int weight = 1);

    /** @brief 移除任务 */
    bool removeTask(const QString& taskId);

    /**
     * @brief 获取下一个任务
     * @return 任务ID(空串表示无任务)
     */
    QString next();

    /**
     * @brief 批量调度
     * @param count 需要调度的次数
     * @return 任务ID列表
     */
    QVector<QString> schedule(int count);

    /** @brief 当前任务数 */
    int taskCount() const;

    /** @brief 设置任务权重 */
    void setWeight(const QString& taskId, int weight);

    /** @brief 获取统计 */
    Stats stats() const;

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 任务被调度信号 */
    void taskScheduled(const QString& taskId, int timesScheduled);

private:
    struct TaskInfo {
        QString id;
        int weight;
        int counter;
        int timesScheduled;
    };

    QVector<TaskInfo> m_tasks;
    int m_currentIndex;

    Stats m_stats;
    double m_timeSum;
};
