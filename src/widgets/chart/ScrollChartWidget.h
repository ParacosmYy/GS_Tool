/**
 * @file ScrollChartWidget.h
 * @brief 轻量级滚动折线图 — 多通道实时数据流可视化
 *
 * 与重量级 ScopeWidget 不同，ScrollChartWidget 针对简单、高速的数据可视化场景优化:
 * CPU使用率、信号电平、传感器读数等。维护固定长度数据窗口，支持多通道叠加、
 * 自动/手动Y轴范围、网格线显示、QPainterPath 高效绘制、PNG快照导出。
 * 所有颜色通过 ThemeManager 语义色获取，禁止硬编码。
 */

#ifndef SCROLLCHARTWIDGET_H
#define SCROLLCHARTWIDGET_H

#include <QWidget>
#include <QVector>
#include <QColor>
#include <QString>
#include <QtGlobal>

class QPaintEvent;

/**
 * @class ScrollChartWidget
 * @brief 轻量级实时滚动折线图控件
 * @details 维护多通道固定长度数据窗口（默认200可见点），addSample() 追加数据自动滚动，
 *          paintEvent 使用 QPainterPath 绘制高效折线。支持自动/手动Y轴范围、
 *          网格线、图例、坐标轴标签、PNG快照导出。纯显示控件，无鼠标交互。
 */
class ScrollChartWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 单个数据通道的完整信息 */
    struct Channel {
        QString name;              ///< 通道名称
        QColor color;              ///< 通道折线颜色
        QVector<double> data;      ///< 历史数据缓冲区
        double minVal = 0.0;       ///< 通道历史最小值
        double maxVal = 0.0;       ///< 通道历史最大值
        bool visible = true;       ///< 是否可见
    };

    /** @brief 运行统计数据结构体，聚合全部运行期间计数器 */
    struct Stats {
        quint64 totalSamplesAdded = 0;      ///< 累计追加样本总数
        quint64 totalPaints = 0;            ///< 累计重绘次数(paintEvent触发)
        quint64 totalChannelChanges = 0;    ///< 累计通道增删次数
        quint64 totalExports = 0;           ///< 累计PNG导出次数
        double  peakSampleRate = 0.0;       ///< 历史峰值采样率(样本/秒)
        int     maxChannels = 0;            ///< 历史最大通道数
    };

    /** @brief 构造滚动折线图控件 @param parent 父控件指针 */
    explicit ScrollChartWidget(QWidget* parent = nullptr);

    /** @brief 添加新数据通道 @param name 通道名称 @param color 折线颜色 @return 通道索引 */
    int addChannel(const QString& name, const QColor& color);

    /** @brief 移除指定索引的数据通道 @param index 通道索引 */
    void removeChannel(int index);

    /** @brief 向指定通道追加一个数据点 @param channelIndex 通道索引 @param value 数据值 */
    void addSample(int channelIndex, double value);

    /** @brief 设置可见数据点数(窗口宽度) @param count 可见点数(>=10) */
    void setVisiblePoints(int count);

    /** @brief 获取当前可见数据点数 @return 可见点数 */
    int visiblePoints() const;

    /** @brief 设置是否自动缩放Y轴 @param enabled true=自动 false=手动 */
    void setAutoScale(bool enabled);

    /** @brief 设置手动Y轴范围(自动缩放关闭时生效) @param min 最小值 @param max 最大值 */
    void setYRange(double min, double max);

    /** @brief 设置是否显示网格线 @param enabled true=显示 */
    void setGridEnabled(bool enabled);

    /** @brief 设置折线宽度 @param width 线宽(像素) */
    void setLineWidth(qreal width);

    /** @brief 获取当前通道数 @return 通道数量 */
    int channelCount() const;

    /** @brief 获取指定通道信息(只读) @param index 通道索引 @return Channel常量引用 */
    const Channel& channelInfo(int index) const;

    /** @brief 导出当前图表为PNG图片 @param filePath 目标文件路径 @param width 图片宽度 @param height 图片高度 @return true导出成功 */
    bool exportToPng(const QString& filePath, int width = 800, int height = 400);

    /** @brief 获取统计数据的只读引用 @return Stats常量引用 */
    const Stats& stats() const;

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /** @brief 新通道已添加 @param index 通道索引 */
    void channelAdded(int index);

    /** @brief 通道已移除 @param index 通道索引 */
    void channelRemoved(int index);

    /** @brief 数据缓冲区溢出(旧数据被裁剪) @param channelIndex 通道索引 */
    void dataOverflow(int channelIndex);

protected:
    /** @brief 绘制事件 — 绘制背景、网格、坐标轴标签、折线、图例 @param event 绘制事件 */
    void paintEvent(QPaintEvent* event) override;

    /** @brief 建议最小尺寸 @return 200x120像素 */
    QSize minimumSizeHint() const override;

private:
    /** @brief 绘制网格线(水平+垂直虚线) @param painter 画笔引用 @param plotArea 绘图区域 */
    void drawGrid(QPainter& painter, const QRectF& plotArea);

    /** @brief 绘制所有可见通道的折线 @param painter 画笔引用 @param plotArea 绘图区域 */
    void drawChannels(QPainter& painter, const QRectF& plotArea);

    /** @brief 绘制Y轴/X轴刻度标签 @param painter 画笔引用 @param plotArea 绘图区域 */
    void drawAxisLabels(QPainter& painter, const QRectF& plotArea);

    /** @brief 计算绘图区域(扣除边距) @return 绘图区域矩形 */
    QRectF computePlotArea() const;

    QVector<Channel> m_channels;       ///< 数据通道列表
    int m_visiblePoints;                ///< 可见数据点数(窗口宽度)
    bool m_autoScale;                   ///< 是否自动缩放Y轴
    double m_yMin;                      ///< 手动Y轴最小值
    double m_yMax;                      ///< 手动Y轴最大值
    bool m_gridEnabled;                 ///< 是否显示网格
    qreal m_lineWidth;                  ///< 折线宽度(像素)
    int m_leftMargin;                   ///< 左边距(留给Y轴标签)
    int m_bottomMargin;                 ///< 底边距(留给X轴标签)
    Stats m_stats;                      ///< 运行统计
};

#endif // SCROLLCHARTWIDGET_H
