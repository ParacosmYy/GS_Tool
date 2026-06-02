/**
 * @file YAxisManager.h
 * @brief 多通道独立Y轴管理器 -- 为每个通道分配独立的QValueAxis
 *
 * 设计要点:
 *   - 每个通道拥有独立的Y轴，颜色与通道线条一致
 *   - 支持左/右两侧布局，最多4个轴（左2右2）
 *   - 超过4个通道时按单位(unit)分组合并轴
 *   - 通过信号通知 ChartWidget 刷新轴布局
 *
 * 协作关系:
 *   - ChartWidget: 拥有 YAxisManager，在通道变化时调用创建/删除轴
 *   - ChannelConfig: 提供 yAxisSide 和 unit 字段
 *   - ThemeManager: 主题切换时更新轴标签颜色
 *   - ChartColors: 轴颜色与通道线条颜色保持一致
 */

#ifndef YAXISMANAGER_H
#define YAXISMANAGER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QColor>
#include <QVector>
#include <QtCharts>

/**
 * @brief Y轴放置位置枚举
 */
enum class YAxisSide {
    Left,   ///< 左侧Y轴
    Right   ///< 右侧Y轴
};

class QChart;

/**
 * @brief 多通道独立Y轴管理器
 *
 * 管理一组 QValueAxis，每个通道（或同单位通道组）对应一个独立Y轴。
 * 负责轴的创建/删除/范围更新/主题颜色应用。
 *
 * 轴分配策略:
 *   - <= 4通道: 每通道独立轴，交替分配左右侧（0,2→左，1,3→右）
 *   - > 4通道: 按单位分组，同单位通道共享一个轴，仍限制左右各最多2轴
 */
class YAxisManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造Y轴管理器
     * @param chart 关联的QChart对象（轴将添加到此图表）
     * @param parent 父对象
     */
    explicit YAxisManager(QChart* chart, QObject* parent = nullptr);

    /** @brief 析构时清理所有轴 */
    ~YAxisManager();

    /**
     * @brief 为通道创建独立Y轴
     *
     * 如果已存在同名通道轴则跳过。轴的颜色设为通道颜色，
     * 位置由 side 参数决定。
     *
     * @param channel 通道名称
     * @param color 通道颜色（轴标签/线条使用此颜色）
     * @param side 左/右侧放置
     * @param unit 单位文本（作为轴标题）
     */
    void createAxis(const QString& channel, const QColor& color,
                    YAxisSide side, const QString& unit = QString());

    /**
     * @brief 移除通道对应的Y轴
     * @param channel 通道名称
     */
    void removeAxis(const QString& channel);

    /**
     * @brief 更新通道Y轴的范围
     * @param channel 通道名称
     * @param min 最小值
     * @param max 最大值
     */
    void updateRange(const QString& channel, double min, double max);

    /**
     * @brief 获取通道对应的QValueAxis
     * @param channel 通道名称
     * @return 轴指针，不存在时返回 nullptr
     */
    QValueAxis* axisForChannel(const QString& channel) const;

    /**
     * @brief 将 series 附加到通道对应的Y轴
     * @param channel 通道名称
     * @param series 要附加的 QLineSeries
     */
    void attachSeries(const QString& channel, QLineSeries* series);

    /**
     * @brief 清除所有Y轴（通道变化时全量重建）
     */
    void clearAll();

    /**
     * @brief 根据通道列表自动分配Y轴左右侧
     *
     * 策略:
     *   - <= 4通道: 交替分配（偶数索引→左，奇数索引→右）
     *   - > 4通道: 按单位分组，左2右2
     *
     * @param channelNames 通道名称列表
     * @param units 对应的单位列表
     * @return 每个通道对应的 YAxisSide
     */
    static QVector<YAxisSide> autoAssignSides(
        const QStringList& channelNames, const QStringList& units);

    /**
     * @brief 应用当前主题颜色到所有Y轴
     *
     * 更新轴标签颜色（使用通道颜色或主题文字色）和网格线颜色。
     * @param gridColor 网格线颜色（来自 ThemeManager::Border）
     * @param labelColor 标签文字颜色（来自 ThemeManager::TextSecondary）
     */
    void applyThemeColors(const QColor& gridColor, const QColor& labelColor);

signals:
    /** @brief Y轴布局变化信号（增删轴后通知 ChartWidget 刷新） */
    void axesChanged();

private:
    /**
     * @brief 获取指定侧已使用的轴数量
     * @param side 左/右侧
     * @return 该侧已有的轴数量
     */
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
};

#endif // YAXISMANAGER_H
