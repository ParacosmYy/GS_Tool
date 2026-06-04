/**
 * @file ScopeWidget.h
 * @brief 示波器组件 - 多通道实时波形显示
 *
 * 职责: 多通道采样缓冲区 / 网格背景波形渲染 / 时基电压刻度调节 / 触发条件 / 单/批量采样输入
 */

#pragma once
#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QPair>

/** @brief 示波器波形显示组件，类似硬件示波器多通道实时渲染。支持时基(ms/div)、电压刻度(V/div)、触发通道/电平，10水平分度网格 */
class ScopeWidget : public QWidget {
    Q_OBJECT
public:
    explicit ScopeWidget(QWidget *parent = nullptr);
    ~ScopeWidget() override;

    void setChannelCount(int count);                              ///< 设置通道数量
    void setSampleBuffer(int size);                               ///< 设置每通道缓冲区大小(采样点数)
    void addSample(int channel, double value);                    ///< 向指定通道添加单个采样值
    void addSamples(int channel, const QVector<double> &values);  ///< 批量添加采样值
    void setTimeScale(double msPerDiv);                           ///< 设置时基刻度(ms/div，默认1.0)
    void setVoltageScale(double voltsPerDiv);                     ///< 设置电压刻度(V/div，默认1.0)
    void setTriggerChannel(int ch);                               ///< 设置触发通道(默认0)
    void setTriggerLevel(double level);                           ///< 设置触发电平阈值(默认0.0)
    void setRunning(bool on);                                     ///< 设置是否持续运行
    void clearData();                                             ///< 清空所有通道数据
    int channelCount() const;                                     ///< 获取当前通道数
    bool isRunning() const;                                       ///< 查询是否运行中

signals:
    void triggerFired();   ///< 触发条件满足时发射
    void dataOverflow();   ///< 采样缓冲区溢出时发射

protected:
    void paintEvent(QPaintEvent *event) override;                ///< 绘制波形和网格
    void resizeEvent(QResizeEvent *event) override;              ///< 窗口大小变更触发重绘

private:
    void drawGrid(QPainter &p, int w, int h);                    ///< 绘制背景网格线

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
    quint64 m_totalPauses = 0;            ///< 总暂停次数
    quint64 m_totalRestarts = 0;          ///< 总重启次数
    quint64 m_totalTriggerFires = 0;      ///< 总触发信号发射次数
    quint64 m_totalPointsRendered = 0;    ///< 总渲染数据点数
    double m_avgRenderTimeMs = 0.0;       ///< 平均渲染耗时(毫秒)

public:
    quint64 totalSamples() const;         ///< 获取总采样点数
    quint64 totalRepaints() const;        ///< 获取总重绘次数
    quint64 totalTriggers() const;        ///< 获取总触发次数
    quint64 totalOverflows() const;       ///< 获取总溢出次数
    quint64 totalClears() const;          ///< 获取总清空次数
    quint64 totalScaleChanges() const;    ///< 获取总刻度变更次数
    quint64 totalChannelChanges() const;  ///< 获取总通道变更次数
    quint64 totalPauses() const;          ///< 获取总暂停次数
    quint64 totalRestarts() const;        ///< 获取总重启次数
    quint64 totalTriggerFires() const;    ///< 获取总触发发射次数
    quint64 totalPointsRendered() const;  ///< 获取总渲染数据点数
    double avgRenderTimeMs() const;       ///< 获取平均渲染耗时(毫秒)
    void resetScopeStatistics();          ///< 重置示波器统计计数器
};
