/**
 * @file DashboardWidget.h
 * @brief 仪表盘主容器控件
 *
 * 提供可动态添加/移除仪表盘组件的网格容器，
 * 支持布局的序列化与反序列化。
 */

#ifndef DASHBOARD_WIDGET_H
#define DASHBOARD_WIDGET_H

#include <QWidget>
#include <QVariantMap>
#include <QGridLayout>

/**
 * @class DashboardWidget
 * @brief 仪表盘主容器 —— 以网格布局管理多个仪表盘子控件
 */
class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    /// 构造函数
    explicit DashboardWidget(QWidget *parent = nullptr);

    /// 析构函数
    ~DashboardWidget() override;

    /**
     * @brief 添加一个仪表盘子组件
     * @param type    组件类型（gauge / progress / led / numeric）
     * @param channel 绑定的数据通道名称
     * @return 新组件的索引，-1 表示失败
     */
    int addComponent(const QString &type, const QString &channel);

    /**
     * @brief 移除指定索引的组件
     * @param index 组件索引
     */
    void removeComponent(int index);

    /**
     * @brief 从 QVariantMap 恢复布局
     * @param layout 布局描述
     */
    void loadLayout(const QVariantMap &layout);

    /**
     * @brief 将当前布局导出为 QVariantMap
     * @return 布局描述
     */
    QVariantMap saveLayout() const;

    /**
     * @brief 获取指定索引的子组件
     * @param index 索引
     * @return 子控件指针，越界返回 nullptr
     */
    QWidget *componentAt(int index) const;

    /**
     * @brief 获取子组件总数
     * @return 数量
     */
    int componentCount() const;

    // ---- 统计计数接口 ----
    /** @brief 获取累计布局变更次数 */
    quint64 totalLayoutChanges() const;
    /** @brief 获取累计添加组件次数 */
    quint64 totalWidgetsAdded() const;
    /** @brief 获取累计移除组件次数 */
    quint64 totalWidgetsRemoved() const;
    /** @brief 重置所有仪表盘容器统计计数器 */
    void resetDashboardWidgetStatistics();

signals:
    /// 布局发生变更时发射
    void layoutChanged();

private:
    /// 初始化 UI 与示例控件
    void setupUI();

    QGridLayout    *m_grid;        ///< 网格布局
    QList<QWidget*> m_components;  ///< 子组件列表
    static const int kColumns = 3; ///< 网格列数

    // ---- 统计计数器 ----
    quint64 m_totalLayoutChanges = 0;   ///< 累计布局变更次数
    quint64 m_totalWidgetsAdded = 0;    ///< 累计添加组件次数
    quint64 m_totalWidgetsRemoved = 0;  ///< 累计移除组件次数
};

#endif // DASHBOARD_WIDGET_H
