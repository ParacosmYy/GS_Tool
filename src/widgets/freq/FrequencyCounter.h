/**
 * @file FrequencyCounter.h
 * @brief 频率计数器组件 - 测量周期信号的频率、周期和脉冲数
 *
 * 职责:
 *   1. 通过可配置的触发电平检测信号边沿
 *   2. 在门控时间内统计脉冲数量并计算频率
 *   3. 实时显示频率值和脉冲计数
 */

#pragma once
#include <QWidget>
#include <QByteArray>
#include <QTimer>
#include <QLabel>

/**
 * @brief 频率计数器组件
 *
 * 支持逐采样点或批量缓冲区方式输入信号数据，
 * 通过触发电平检测上升沿，在门控时间窗口内计算信号频率。
 */
class FrequencyCounter : public QWidget {
    Q_OBJECT
public:
    /**
     * @brief 构造频率计数器
     * @param parent 父widget
     */
    explicit FrequencyCounter(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~FrequencyCounter() override;

    /**
     * @brief 输入单个采样值
     * @param value 采样值（幅度）
     */
    void feedSample(double value);

    /**
     * @brief 批量输入采样缓冲区
     * @param data 原始字节数据
     * @param sampleSize 每个采样点的字节宽度，默认2（16位）
     */
    void feedBuffer(const QByteArray &data, int sampleSize = 2);

    /**
     * @brief 设置门控时间
     * @param msecs 门控时间（毫秒），默认1000
     */
    void setGateTime(int msecs);

    /**
     * @brief 设置触发电平
     * @param level 触发电平阈值，默认0.0
     */
    void setTriggerLevel(double level);

    /** @brief 重置计数器状态 */
    void reset();

    /**
     * @brief 获取当前测量频率
     * @return 频率值（Hz）
     */
    double frequency() const;

    /**
     * @brief 获取信号周期
     * @return 周期值（毫秒）
     */
    double period() const;

    /**
     * @brief 获取累计脉冲数
     * @return 脉冲计数
     */
    int pulseCount() const;

signals:
    /** @brief 频率测量值更新 @param hz 新的频率值（Hz） */
    void frequencyChanged(double hz);

    /** @brief 检测到新脉冲 @param count 当前累计脉冲数 */
    void pulseCounted(int count);

protected:
    /** @brief 初始化UI布局 */
    void setupUi();

private:
    /** @brief 门控定时器超时回调，结算本窗口内的频率 */
    void onGateTimeout();

    QLabel *m_freqLabel = nullptr;      ///< 频率显示标签
    QLabel *m_countLabel = nullptr;     ///< 脉冲计数显示标签
    double m_frequency = 0.0;           ///< 当前测量频率（Hz）
    int m_pulseCount = 0;               ///< 累计脉冲数
    int m_gateTime = 1000;              ///< 门控时间（毫秒）
    double m_triggerLevel = 0.0;        ///< 触发电平阈值
    bool m_lastAbove = false;           ///< 上一个采样点是否高于触发电平
    QTimer *m_gateTimer = nullptr;      ///< 门控定时器
    qint64 m_gateStart = 0;             ///< 当前门控窗口起始时间戳
};
