/**
 * @file ScopeWidget.h
 * @brief 示波器组件 - 多通道实时波形显示
 *
 * 职责:
 *   1. 管理多通道采样数据缓冲区
 *   2. 以网格背景绘制示波器风格波形
 *   3. 支持时基/电压刻度调节和触发条件设置
 *   4. 支持单通道和批量采样数据输入
 */

#pragma once
#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QPair>

/**
 * @brief 示波器波形显示组件
 *
 * 提供类似硬件示波器的多通道波形实时渲染。
 * 支持可配置的时基（ms/div）、电压刻度（V/div）、
 * 触发通道和触发电平，网格分为10个水平分度。
 */
class ScopeWidget : public QWidget {
    Q_OBJECT
public:
    /**
     * @brief 构造示波器组件
     * @param parent 父widget
     */
    explicit ScopeWidget(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~ScopeWidget() override;

    /**
     * @brief 设置通道数量
     * @param count 通道数
     */
    void setChannelCount(int count);

    /**
     * @brief 设置每个通道的采样缓冲区大小
     * @param size 缓冲区长度（采样点数）
     */
    void setSampleBuffer(int size);

    /**
     * @brief 向指定通道添加单个采样值
     * @param channel 通道索引
     * @param value 采样值
     */
    void addSample(int channel, double value);

    /**
     * @brief 向指定通道批量添加采样值
     * @param channel 通道索引
     * @param values 采样值数组
     */
    void addSamples(int channel, const QVector<double> &values);

    /**
     * @brief 设置时基刻度
     * @param msPerDiv 每分度对应的毫秒数，默认1.0
     */
    void setTimeScale(double msPerDiv);

    /**
     * @brief 设置电压刻度
     * @param voltsPerDiv 每分度对应的电压值，默认1.0
     */
    void setVoltageScale(double voltsPerDiv);

    /**
     * @brief 设置触发通道
     * @param ch 触发源通道索引，默认0
     */
    void setTriggerChannel(int ch);

    /**
     * @brief 设置触发电平
     * @param level 触发电平阈值，默认0.0
     */
    void setTriggerLevel(double level);

    /**
     * @brief 设置是否持续运行
     * @param on true运行，false停止
     */
    void setRunning(bool on);

    /** @brief 清空所有通道数据 */
    void clearData();

    /**
     * @brief 获取当前通道数量
     * @return 通道数
     */
    int channelCount() const;

    /**
     * @brief 查询是否正在运行
     * @return true表示运行中
     */
    bool isRunning() const;

signals:
    /** @brief 触发条件被满足时发射 */
    void triggerFired();

    /** @brief 采样缓冲区溢出时发射 */
    void dataOverflow();

protected:
    /** @brief 绘制波形和网格 */
    void paintEvent(QPaintEvent *event) override;

    /** @brief 窗口大小变更时触发重绘 */
    void resizeEvent(QResizeEvent *event) override;

private:
    /**
     * @brief 绘制背景网格线
     * @param p 画笔
     * @param w 绘制区域宽度
     * @param h 绘制区域高度
     */
    void drawGrid(QPainter &p, int w, int h);

    QVector<QVector<double>> m_channels; ///< 多通道采样数据缓冲区
    int m_bufferSize = 1024;             ///< 每通道缓冲区长度
    double m_timeScale = 1.0;            ///< 时基刻度（ms/div）
    double m_voltScale = 1.0;            ///< 电压刻度（V/div）
    int m_triggerCh = 0;                 ///< 触发源通道索引
    double m_triggerLevel = 0.0;         ///< 触发电平阈值
    bool m_running = true;               ///< 运行状态标志
    int m_writePos = 0;                  ///< 环形缓冲区写入位置
    bool m_wrapped = false;              ///< 环形缓冲区是否已写满一轮
    static constexpr int kDivisions = 10; ///< 水平分度数

    // ---- 统计计数器 ----
    quint64 m_totalSamples = 0;           ///< 总采样点数
    quint64 m_totalRepaints = 0;          ///< 总重绘次数
    quint64 m_totalTriggers = 0;          ///< 总触发次数
    quint64 m_totalOverflows = 0;         ///< 总溢出次数
    quint64 m_totalClears = 0;            ///< 总数据清空次数
    quint64 m_totalScaleChanges = 0;      ///< 总刻度变更次数
    quint64 m_totalChannelChanges = 0;    ///< 总通道数变更次数
    quint64 m_totalPauses = 0;            ///< 总暂停次数(setRunning false)
    quint64 m_totalRestarts = 0;          ///< 总重启次数(setRunning true)
    quint64 m_totalTriggerFires = 0;      ///< 总触发信号发射次数

public:
    /** @brief 获取总采样点数 @return 累计采样点 */
    quint64 totalSamples() const { return m_totalSamples; }
    /** @brief 获取总重绘次数 @return 累计重绘 */
    quint64 totalRepaints() const { return m_totalRepaints; }
    /** @brief 获取总触发次数 @return 累计触发 */
    quint64 totalTriggers() const { return m_totalTriggers; }
    /** @brief 获取总溢出次数 @return 累计溢出 */
    quint64 totalOverflows() const { return m_totalOverflows; }
    /** @brief 获取总数据清空次数 @return 累计清空次数 */
    quint64 totalClears() const { return m_totalClears; }
    /** @brief 获取总刻度变更次数 @return 累计刻度变更次数 */
    quint64 totalScaleChanges() const { return m_totalScaleChanges; }
    /** @brief 获取总通道数变更次数 @return 累计通道数变更次数 */
    quint64 totalChannelChanges() const { return m_totalChannelChanges; }
    /** @brief 获取总暂停次数 @return 累计暂停次数 */
    quint64 totalPauses() const { return m_totalPauses; }
    /** @brief 获取总重启次数 @return 累计重启次数 */
    quint64 totalRestarts() const { return m_totalRestarts; }
    /** @brief 获取总触发信号发射次数 @return 累计触发发射次数 */
    quint64 totalTriggerFires() const { return m_totalTriggerFires; }
    /** @brief 重置示波器统计计数器 */
    void resetScopeStatistics() { m_totalSamples = 0; m_totalRepaints = 0; m_totalTriggers = 0; m_totalOverflows = 0; m_totalClears = 0; m_totalScaleChanges = 0; m_totalChannelChanges = 0; m_totalPauses = 0; m_totalRestarts = 0; m_totalTriggerFires = 0; }
};
