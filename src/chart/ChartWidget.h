#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QtCharts>
#include <QMap>
#include <QColor>
#include <QString>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

#include "chart/ChannelConfig.h"
#include "chart/ChartModel.h"

struct FrameDefinition;

// 实时波形图控件 - 基于Qt Charts的滑动窗口波形显示
// 支持多通道同时显示，每个通道一条曲线
// 数据层由ChartModel管理，本类只负责渲染
class ChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartWidget(QWidget* parent = nullptr);

    // 获取内部ChartModel，用于外部连接信号
    ChartModel* model() const;

    // 从帧定义自动生成通道配置并应用
    void configureFromFrameDefinition(const FrameDefinition& def);

    // 设置滑动窗口大小（显示的最大数据点数）
    void setWindowSize(int points);

    // 清除所有通道数据
    void clear();

    // 获取当前通道列表
    QStringList channels() const;

    // 设置Y轴范围
    void setYRange(double min, double max);

    // 启用/禁用自动Y轴范围
    void setAutoYRange(bool enabled);

public slots:
    // 从帧解析结果中自动提取数值字段并添加到对应通道（兼容旧接口）
    void onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

private slots:
    // ChartModel::dataUpdated 触发的渲染刷新
    void updateChart(const QStringList& updatedChannels);

    // ChartModel::channelsChanged 触发的通道重建
    void onChannelsChanged();

    // ChartModel::dataCleared 触发的清除
    void onDataCleared();

    void onPauseToggled(bool paused);
    void onClearClicked();

private:
    void setupUI();

    // 为指定通道创建QLineSeries并添加到图表
    void createSeries(const QString& name, const QColor& color);

    // 移除指定通道的QLineSeries
    void removeSeries(const QString& name);

    QChartView* m_chartView;
    QChart* m_chart;
    QValueAxis* m_xAxis;
    QValueAxis* m_yAxis;

    // 数据模型（管理通道配置、滑动窗口、降采样）
    ChartModel* m_model;
    ChannelConfigSet m_configSet;

    // 通道名 → QLineSeries 映射（仅用于渲染层）
    QMap<QString, QLineSeries*> m_seriesMap;

    bool m_autoYRange = true;

    // 控制栏
    QPushButton* m_pauseBtn;
    QPushButton* m_clearBtn;
    QComboBox* m_windowSizeCombo;
    QLabel* m_statusLabel;
    bool m_paused = false;

    // 预定义颜色表
    static const QVector<QColor> kDefaultColors;
};

#endif // CHARTWIDGET_H
