/**
 * @file DashboardWidget.cpp
 * @brief 仪表盘主容器控件实现
 */

#include "dashboard/DashboardWidget.h"

/**
 * @brief 构造函数，初始化 UI 与成员
 * @param parent 父控件
 */
DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent)
    , m_grid(nullptr)
{
    setObjectName("DashboardWidget");
    setupUI();
}

/** @brief 析构函数 */
DashboardWidget::~DashboardWidget() = default;

/**
 * @brief 添加一个仪表盘子组件
 * @param type    组件类型
 * @param channel 绑定数据通道
 * @return 组件索引，暂返回 0
 */
int DashboardWidget::addComponent(const QString &type, const QString &channel)
{
    Q_UNUSED(type)
    Q_UNUSED(channel)
    // TODO: 根据 type 创建对应子控件并加入 m_grid
    return 0;
}

/**
 * @brief 移除指定索引的组件
 * @param index 组件索引
 */
void DashboardWidget::removeComponent(int index)
{
    Q_UNUSED(index)
    // TODO: 从 m_grid 和 m_components 中移除
}

/**
 * @brief 从 QVariantMap 恢复布局
 * @param layout 布局描述
 */
void DashboardWidget::loadLayout(const QVariantMap &layout)
{
    Q_UNUSED(layout)
    // TODO: 解析 layout 并重建子控件
}

/**
 * @brief 导出当前布局
 * @return 空的 QVariantMap（暂未实现）
 */
QVariantMap DashboardWidget::saveLayout() const
{
    return QVariantMap();
}

/** @brief 初始化 UI：创建网格布局 */
void DashboardWidget::setupUI()
{
    m_grid = new QGridLayout(this);
    setLayout(m_grid);
}
