/**
 * @file FractionalDelay.h
 * @brief 分数延迟滤波器 — Thiran全通插值
 *
 * 功能: 实现分数延迟(Fractional Delay)滤波器，使用Thiran全通
 *       插值器实现亚采样精度的延迟，保持信号的频率响应。
 *       支持可变延迟、多阶全通滤波器和延迟调制。
 *       适用于音频处理、通信系统定时恢复和波束成形。
 *
 * 协作: AdaptiveFilter(自适应滤波) / FftEngine(频域分析)
 */
#pragma once

#include <QObject>
#include <QVector>

#include <vector>

/**
 * @brief 分数延迟滤波器 — Thiran全通插值
 */
class FractionalDelay : public QObject {
    Q_OBJECT

public:
    /** @brief 统计信息 */
    struct Stats {
        int totalSamplesProcessed = 0;  ///< 累计处理采样数
        int totalDelayChanges = 0;      ///< 累计延迟变更次数
        int totalResets = 0;            ///< 累计重置次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    /**
     * @brief 构造函数
     * @param order 全通滤波器阶数(1~8)
     * @param maxDelay 最大延迟(采样点)
     * @param parent 父对象
     */
    explicit FractionalDelay(int order = 4, int maxDelay = 4096,
                             QObject* parent = nullptr);

    /**
     * @brief 设置延迟值
     * @param delay 延迟(采样点，支持小数)
     */
    void setDelay(double delay);

    /**
     * @brief 处理单个采样点
     * @param input 输入采样
     * @return 延迟后的采样
     */
    double processOne(double input);

    /**
     * @brief 处理采样缓冲区
     * @param input 输入缓冲区
     * @return 延迟后的缓冲区
     */
    QVector<double> process(const QVector<double>& input);

    /**
     * @brief 处理STL向量缓冲区
     * @param input 输入缓冲区
     * @return 延迟后的缓冲区
     */
    std::vector<double> process(const std::vector<double>& input);

    /**
     * @brief 重置滤波器状态(清空延迟线)
     */
    void reset();

    /**
     * @brief 获取当前延迟值
     * @return 延迟(采样点)
     */
    double currentDelay() const { return m_currentDelay; }

    /**
     * @brief 获取当前整数延迟部分
     * @return 整数延迟
     */
    int integerDelay() const { return m_intDelay; }

    /**
     * @brief 获取当前分数延迟部分
     * @return 分数延迟(0~1)
     */
    double fractionalDelay() const { return m_fracDelay; }

    /**
     * @brief 获取滤波器阶数
     * @return 阶数
     */
    int order() const { return m_order; }

    /**
     * @brief 计算群延迟响应(在某频率处)
     * @param normalizedFreq 归一化频率(0~0.5)
     * @return 群延迟(采样点)
     */
    double groupDelayAt(double normalizedFreq) const;

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计信息 */
    void resetStatistics();

signals:
    /** @brief 延迟变更 @param newDelay 新延迟值 */
    void delayChanged(double newDelay);

private:
    /**
     * @brief 计算Thiran全通滤波器系数
     * @param delay 分数延迟(0~1)
     * @param order 阶数
     * @return 滤波器系数a[0..order]
     */
    static std::vector<double> computeThiranCoeffs(double delay, int order);

    /**
     * @brief 更新内部延迟线和系数
     */
    void updateCoefficients();

    int m_order;                    ///< 全通滤波器阶数
    int m_maxDelay;                 ///< 最大延迟
    double m_currentDelay;          ///< 当前总延迟
    int m_intDelay;                 ///< 整数延迟部分
    double m_fracDelay;             ///< 分数延迟部分

    std::vector<double> m_thiranA;  ///< Thiran全通系数
    std::vector<double> m_delayLine; ///< 整数延迟线
    std::vector<double> m_allpassState; ///< 全通滤波器状态
    int m_delayPos;                 ///< 延迟线写入位置

    Stats m_stats;                  ///< 统计信息
    double m_timeSum = 0.0;         ///< 累计耗时
};
