/**
 * @file YAxisManager.h
 * @brief 多通道独立Y轴管理器 -- 为每个通道分配独立的QValueAxis
 *
 * 每个通道拥有独立的Y轴，颜色与通道线条一致。支持左/右两侧布局，最多4个轴（左2右2）。
 * 超过4个通道时按单位(unit)分组合并轴。通过信号通知ChartWidget刷新轴布局。
 * 协作: ChartWidget(拥有者), ChannelConfig(yAxisSide/unit), ThemeManager(主题色), ChartColors(轴颜色)
 */

#ifndef YAXISMANAGER_H
#define YAXISMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QColor>
#include <QVector>
#include <QtCharts>

/** @brief Y轴放置位置枚举 */
enum class YAxisSide { Left, Right };

class QChart;

/** @brief 多通道独立Y轴管理器，管理QValueAxis的创建/删除/范围更新/主题颜色应用 */
class YAxisManager : public QObject {
    Q_OBJECT

public:
    /** @brief 构造Y轴管理器，关联QChart对象 @param chart QChart指针 @param parent 父对象 */
    explicit YAxisManager(QChart* chart, QObject* parent = nullptr);
    /** @brief 析构时清理所有轴 */
    ~YAxisManager();

    /** @brief 为通道创建独立Y轴 @param channel 通道名称 @param color 轴标签颜色 @param side 放置位置(左/右) @param unit 单位文本 */
    void createAxis(const QString& channel, const QColor& color,
                    YAxisSide side, const QString& unit = QString());
    /** @brief 移除通道对应的Y轴 @param channel 通道名称 */
    void removeAxis(const QString& channel);
    /** @brief 更新通道Y轴的范围 @param channel 通道名称 @param min 最小值 @param max 最大值 */
    void updateRange(const QString& channel, double min, double max);
    /** @brief 获取通道对应的QValueAxis @param channel 通道名称 @return 轴指针，不存在返回nullptr */
    QValueAxis* axisForChannel(const QString& channel) const;
    /** @brief 将series附加到通道对应的Y轴 @param channel 通道名称 @param series 线条序列 */
    void attachSeries(const QString& channel, QLineSeries* series);
    /** @brief 清除所有Y轴（通道变化时全量重建） */
    void clearAll();

    /** @brief 根据通道列表自动分配Y轴左右侧: <=4通道交替分配，>4通道按单位分组 */
    static QVector<YAxisSide> autoAssignSides(
        const QStringList& channelNames, const QStringList& units);

    /** @brief 应用当前主题颜色到所有Y轴，更新轴标签颜色和网格线颜色 */
    void applyThemeColors(const QColor& gridColor, const QColor& labelColor);

    // ---- 统计计数接口 ----
    /** @brief 获取累计缩放重算次数 @return 重算次数 */
    quint64 totalRescales() const;
    /** @brief 获取累计自动缩放事件次数 @return 自动缩放次数 */
    quint64 totalAutoScaleEvents() const;
    /** @brief 获取累计轴添加次数 @return 添加次数 */
    quint64 totalAxisAdds() const { return m_totalAxisAdds; }
    /** @brief 获取累计轴移除次数 @return 移除次数 */
    quint64 totalAxisRemoves() const { return m_totalAxisRemoves; }
    /** @brief 重置所有Y轴统计计数器 */
    void resetYAxisStatistics();

signals:
    /** @brief Y轴布局变化信号（增删轴后通知ChartWidget刷新） */
    void axesChanged();

private:
    /** @brief 获取指定侧已使用的轴数量 @param side 左/右侧 @return 轴数量 */
    int countAxesOnSide(YAxisSide side) const;

    /** @brief 轴信息结构体 */
    struct AxisInfo {
        QValueAxis* axis;       ///< Qt值轴对象
        YAxisSide side;         ///< 放置位置
        QColor color;           ///< 通道颜色（轴标签用此颜色）
        QString unit;           ///< 单位文本
    };

    QChart* m_chart;                               ///< 关联的图表对象
    QMap<QString, AxisInfo> m_axes;                ///< 通道名 → 轴信息映射

    // ---- 统计计数器 ----
    quint64 m_totalRescales = 0;                   ///< 累计缩放重算次数
    quint64 m_totalAutoScaleEvents = 0;            ///< 累计自动缩放事件次数
    quint64 m_totalAxisAdds = 0;                   ///< 累计轴添加次数
    quint64 m_totalAxisRemoves = 0;                ///< 累计轴移除次数
};

#endif // YAXISMANAGER_H
