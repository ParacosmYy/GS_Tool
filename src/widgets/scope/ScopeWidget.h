/**
 * @file ScopeWidget.h
 * @brief 示波器Widget — 多通道实时波形显示组件
 *
 * 模拟硬件示波器功能: 多通道环形缓冲区采样 + 10分度网格背景 +
 * 时基(ms/div)/电压(V/div)刻度调节 + 触发条件检测。
 *
 * 数据流:
 *   - addSample()/addSamples() 写入环形缓冲区
 *   - paintEvent() 读取缓冲区绘制多通道波形
 *   - 触发检测在 ch==0 写入时执行
 *
 * 安全约束:
 *   - setSampleBuffer(0) 自动修正为1，防止模零崩溃
 *   - setChannelCount(0) 自动修正为1
 *   - m_voltScale 为0时绘制使用默认值1.0
 *
 * 统计: 采样点/重绘/触发/溢出/清空/刻度变更/通道变更/暂停/重启/渲染点数/平均渲染耗时
 */

#pragma once
#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QPair>

/**
 * @brief 示波器波形显示组件
 *
 * 类似硬件示波器多通道实时渲染。支持时基(ms/div)、电压刻度(V/div)、
 * 触发通道/电平设置，10水平分度网格。
 *
 * 设计模式: 独立Widget — 所有数据通过addSample输入，paintEvent自动渲染。
 */
class ScopeWidget : public QWidget {
    Q_OBJECT
public:
    /** @brief 构造示波器Widget，默认2通道 @param parent 父Widget */
    explicit ScopeWidget(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~ScopeWidget() override;

    /**
     * @brief 设置通道数量
     * @param count 通道数(最小1，传入0自动修正)
     */
    void setChannelCount(int count);

    /**
     * @brief 设置每通道缓冲区大小
     * @param size 采样点数(最小1，传入0自动修正)
     */
    void setSampleBuffer(int size);

    /**
     * @brief 向指定通道添加单个采样值
     * @param channel 通道索引(必须 < channelCount())
     * @param value 采样值
     */
    void addSample(int channel, double value);

    /** @brief 批量添加采样值 @param channel 通道索引 @param values 采样值向量 */
    void addSamples(int channel, const QVector<double> &values);

    /** @brief 设置时基刻度 @param msPerDiv ms/div(默认1.0) */
    void setTimeScale(double msPerDiv);

    /** @brief 设置电压刻度 @param voltsPerDiv V/div(默认1.0) */
    void setVoltageScale(double voltsPerDiv);

    /** @brief 设置触发源通道 @param ch 通道索引(默认0) */
    void setTriggerChannel(int ch);

    /** @brief 设置触发电平阈值 @param level 触发电平(默认0.0) */
    void setTriggerLevel(double level);

    /** @brief 设置运行状态 @param on true=运行，false=暂停 */
    void setRunning(bool on);

    /** @brief 清空所有通道数据和写入指针 */
    void clearData();

    /** @brief 获取当前通道数 @return 通道数量 */
    int channelCount() const;

    /** @brief 查询运行状态 @return true=运行中 */
    bool isRunning() const;

signals:
    /** @brief 触发条件满足时发射(触发电平匹配) */
    void triggerFired();

    /** @brief 采样缓冲区溢出(写满一轮)时发射 */
    void dataOverflow();

protected:
    void paintEvent(QPaintEvent *event) override;    ///< 绘制波形和网格
    void resizeEvent(QResizeEvent *event) override;  ///< 窗口大小变更触发重绘

private:
    /** @brief 绘制背景点状网格线和中心十字线 */
    void drawGrid(QPainter &p, int w, int h);

    QVector<QVector<double>> m_channels; ///< 多通道采样数据环形缓冲区
    int m_bufferSize = 1024;             ///< 每通道缓冲区长度(最小1)
    double m_timeScale = 1.0;            ///< 时基刻度（ms/div）
    double m_voltScale = 1.0;            ///< 电压刻度（V/div）
    int m_triggerCh = 0;                 ///< 触发源通道索引
    double m_triggerLevel = 0.0;         ///< 触发电平阈值
    bool m_running = true;               ///< 运行状态标志
    int m_writePos = 0;                  ///< 环形缓冲区当前写入位置
    bool m_wrapped = false;              ///< 环形缓冲区是否已写满至少一轮
    static constexpr int kDivisions = 10; ///< 水平/垂直分度数

    // ---- 统计计数器 ----
    quint64 m_totalSamples = 0;           ///< 累计采样点总数
    quint64 m_totalRepaints = 0;          ///< 累计重绘次数
    quint64 m_totalTriggers = 0;          ///< 累计触发次数
    quint64 m_totalOverflows = 0;         ///< 累计缓冲区溢出次数
    quint64 m_totalClears = 0;            ///< 累计数据清空次数
    quint64 m_totalScaleChanges = 0;      ///< 累计刻度变更次数(时基+电压)
    quint64 m_totalChannelChanges = 0;    ///< 累计通道数变更次数
    quint64 m_totalPauses = 0;            ///< 累计暂停次数
    quint64 m_totalRestarts = 0;          ///< 累计从暂停恢复次数
    quint64 m_totalTriggerFires = 0;      ///< 累计触发信号发射次数
    quint64 m_totalPointsRendered = 0;    ///< 累计渲染数据点总数
    double m_avgRenderTimeMs = 0.0;       ///< 平均渲染耗时(EMA, alpha=0.1)

public:
    /** @brief 获取累计采样点总数 */
    quint64 totalSamples() const;
    /** @brief 获取累计重绘次数 */
    quint64 totalRepaints() const;
    /** @brief 获取累计触发次数 */
    quint64 totalTriggers() const;
    /** @brief 获取累计缓冲区溢出次数 */
    quint64 totalOverflows() const;
    /** @brief 获取累计数据清空次数 */
    quint64 totalClears() const;
    /** @brief 获取累计刻度变更次数 */
    quint64 totalScaleChanges() const;
    /** @brief 获取累计通道数变更次数 */
    quint64 totalChannelChanges() const;
    /** @brief 获取累计暂停次数 */
    quint64 totalPauses() const;
    /** @brief 获取累计从暂停恢复次数 */
    quint64 totalRestarts() const;
    /** @brief 获取累计触发信号发射次数 */
    quint64 totalTriggerFires() const;
    /** @brief 获取累计渲染数据点总数 */
    quint64 totalPointsRendered() const;
    /** @brief 获取平均渲染耗时(毫秒) */
    double avgRenderTimeMs() const;
    /** @brief 重置所有示波器统计计数器为零 */
    void resetScopeStatistics();
};
