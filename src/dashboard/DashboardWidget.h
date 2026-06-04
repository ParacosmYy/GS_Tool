/**
 * @file DashboardWidget.h
 * @brief 仪表盘主容器控件 — 网格布局管理 + 完整属性序列化
 *
 * 支持gauge(min/max)/progress(min/max)/led(on/color)/numeric(unit/precision)
 * 四种组件类型的完整属性序列化到JSON/QSettings配置文件。
 */

#ifndef DASHBOARD_WIDGET_H
#define DASHBOARD_WIDGET_H

#include <QWidget>
#include <QVariantMap>
#include <QGridLayout>
#include "dashboard/DashboardSerializer.h"

/**
 * @brief 仪表盘主容器 — 网格布局管理多个仪表盘子控件
 * @details 支持saveLayout()/loadLayout()(QVariantMap)和
 * saveToItems()/loadFromItems()(DashboardItemConfig完整属性)两种序列化方式，
 * 通过serializer()访问底层DashboardSerializer进行文件/配置文件操作。
 */
class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget *parent = nullptr);  ///< 构造函数
    ~DashboardWidget() override;                          ///< 析构函数

    int addComponent(const QString &type, const QString &channel); ///< 添加子组件，返回索引(-1失败)
    void removeComponent(int index);                               ///< 移除指定索引组件

    void loadLayout(const QVariantMap &layout);       ///< 从QVariantMap恢复布局(简单)
    QVariantMap saveLayout() const;                   ///< 导出到QVariantMap(简单)

    QList<DashboardItemConfig> saveToItems() const;   ///< 导出到DashboardItemConfig(完整属性)
    void loadFromItems(const QList<DashboardItemConfig> &items, int columns); ///< 从配置恢复(完整)
    DashboardSerializer* serializer() const;          ///< 获取序列化器实例

    bool saveToFile(const QString &filePath, const QString &name);   ///< 保存到JSON文件
    bool loadFromFile(const QString &filePath);                      ///< 从JSON文件加载
    bool saveToProfile(const QString &profileName);                  ///< 保存到QSettings配置
    bool loadFromProfile(const QString &profileName);                ///< 从QSettings配置加载

    QWidget *componentAt(int index) const;  ///< 获取指定索引子组件
    int componentCount() const;             ///< 获取子组件总数
    int gridColumns() const;                ///< 获取网格列数

    // ---- 统计接口 ----
    quint64 totalLayoutChanges() const;    ///< 累计布局变更次数
    quint64 totalWidgetsAdded() const;     ///< 累计添加组件次数
    quint64 totalWidgetsRemoved() const;   ///< 累计移除组件次数
    quint64 totalFullSaves() const;        ///< 累计完整序列化保存次数
    quint64 totalFullLoads() const;        ///< 累计完整序列化加载次数

    /** @brief 获取累计组件更新次数(setValue/bindChannel触发) @return 更新总数 */
    quint64 totalUpdates() const;
    /** @brief 获取累计值变更通知次数(valueChanged信号触发) @return 变更总数 */
    quint64 totalValueChanged() const;
    /** @brief 获取累计范围变更次数(setRange触发) @return 范围变更总数 */
    quint64 totalRangeChanges() const;
    /** @brief 获取累计重绘次数(paintEvent触发) @return 重绘总数 */
    quint64 totalRepaints() const;

    void resetDashboardWidgetStatistics(); ///< 重置所有统计计数器

signals:
    void layoutChanged();                               ///< 布局发生变更
    void savedToFile(const QString &filePath);          ///< 保存到文件完成
    void loadedFromFile(const QString &filePath);       ///< 从文件加载完成
    void savedToProfile(const QString &profileName);    ///< 保存到配置文件完成
    void loadedFromProfile(const QString &profileName); ///< 从配置文件加载完成

protected:
    /** @brief 重绘事件，累计重绘计数 @param event 绘制事件 */
    void paintEvent(QPaintEvent *event) override;

private:
    void setupUI();  ///< 初始化UI与示例控件

    /// @brief 从QWidget提取完整属性到DashboardItemConfig
    DashboardItemConfig extractWidgetConfig(QWidget *widget, int index) const;
    /// @brief 根据DashboardItemConfig创建并配置单个组件
    QWidget* createWidgetFromConfig(const DashboardItemConfig &config);

    QGridLayout    *m_grid;        ///< 网格布局
    QList<QWidget*> m_components;  ///< 子组件列表
    static const int kColumns = 3; ///< 网格列数

    DashboardSerializer *m_serializer; ///< 序列化器实例

    // ---- 统计计数器 ----
    quint64 m_totalLayoutChanges = 0;
    quint64 m_totalWidgetsAdded = 0;
    quint64 m_totalWidgetsRemoved = 0;
    quint64 m_totalFullSaves = 0;
    quint64 m_totalFullLoads = 0;
    quint64 m_totalUpdates = 0;            ///< 累计组件更新次数
    quint64 m_totalValueChanged = 0;       ///< 累计值变更通知次数
    quint64 m_totalRangeChanges = 0;       ///< 累计范围变更次数
    quint64 m_totalRepaints = 0;           ///< 累计重绘次数
};

#endif // DASHBOARD_WIDGET_H
