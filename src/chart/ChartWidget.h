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

// 实时波形图控件 - 基于Qt Charts的滑动窗口波形显示
// 支持多通道同时显示，每个通道一条曲线
class ChartWidget : public QWidget {
    Q_OBJECT

public:
    explicit ChartWidget(QWidget* parent = nullptr);

    // 添加一个数据通道
    void addChannel(const QString& name, const QColor& color = QColor());

    // 移除一个数据通道
    void removeChannel(const QString& name);

    // 向指定通道追加数据点
    void appendData(const QString& channel, double value);

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
    // 从帧解析结果中自动提取数值字段并添加到对应通道
    void onFrameParsed(const QVariantMap& fields, const QByteArray& rawFrame);

private slots:
    void onPauseToggled(bool paused);
    void onClearClicked();

private:
    void setupUI();

    // 单个通道的数据
    struct ChannelData {
        QLineSeries* series;
        QVector<QPointF> points;
        double yMin = 0;
        double yMax = 0;
    };

    QChartView* m_chartView;
    QChart* m_chart;
    QValueAxis* m_xAxis;
    QValueAxis* m_yAxis;

    QMap<QString, ChannelData> m_channels;
    int m_windowSize = 200;     // 默认显示200个点
    int m_xCounter = 0;         // X轴计数器
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
