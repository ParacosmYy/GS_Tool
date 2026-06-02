/**
 * @file DashboardWidget.cpp
 * @brief 仪表盘主容器控件实现
 *
 * 以 QGridLayout(3列) 管理仪表盘子组件。
 * 构造时添加 6 个示例控件（2×Gauge、1×ProgressBar、1×LED、2×NumericDisplay），
 * 支持动态增删与布局的序列化存根。
 */

#include "dashboard/DashboardWidget.h"
#include "dashboard/GaugeWidget.h"
#include "dashboard/ProgressBarWidget.h"
#include "dashboard/LedIndicatorWidget.h"
#include "dashboard/NumericDisplayWidget.h"

/**
 * @brief 构造函数，初始化 UI 与示例控件
 * @param parent 父控件
 */
DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent)
    , m_grid(nullptr)
{
    setObjectName("DashboardWidget");
    setupUI();
}

/** @brief 析构函数 —— 子控件由 Qt 父子树自动销毁 */
DashboardWidget::~DashboardWidget() = default;

/**
 * @brief 添加一个仪表盘子组件
 * @param type    组件类型: "gauge" / "progress" / "led" / "numeric"
 * @param channel 绑定数据通道
 * @return 组件索引，失败返回 -1
 */
int DashboardWidget::addComponent(const QString &type, const QString &channel)
{
    QWidget *widget = nullptr;

    if (type == QLatin1String("gauge")) {
        auto *g = new GaugeWidget(this);
        g->setLabel(channel.isEmpty() ? tr("未命名") : channel);
        g->bindChannel(channel);
        g->setRange(0.0, 100.0);
        g->setValue(42.0);
        widget = g;
    } else if (type == QLatin1String("progress")) {
        auto *p = new ProgressBarWidget(this);
        p->setLabel(channel.isEmpty() ? tr("进度") : channel);
        p->bindChannel(channel);
        p->setRange(0.0, 100.0);
        p->setValue(65.0);
        widget = p;
    } else if (type == QLatin1String("led")) {
        auto *led = new LedIndicatorWidget(this);
        led->setOn(true);
        led->setColor(Qt::green);
        led->bindChannel(channel);
        widget = led;
    } else if (type == QLatin1String("numeric")) {
        auto *n = new NumericDisplayWidget(this);
        n->bindChannel(channel);
        n->setUnit(tr("V"));
        n->setPrecision(2);
        n->setValue(3.30);
        widget = n;
    }

    if (!widget) {
        return -1;
    }

    int index = m_components.size();
    int row = index / kColumns;
    int col = index % kColumns;
    m_grid->addWidget(widget, row, col);
    m_components.append(widget);

    emit layoutChanged();
    return index;
}

/**
 * @brief 移除指定索引的组件
 * @param index 组件索引
 */
void DashboardWidget::removeComponent(int index)
{
    if (index < 0 || index >= m_components.size()) {
        return;
    }

    QWidget *w = m_components.takeAt(index);
    m_grid->removeWidget(w);
    delete w;

    /* 重新排列网格：清除剩余，重新添加 */
    for (int i = 0; i < m_components.size(); ++i) {
        m_grid->removeWidget(m_components[i]);
    }
    for (int i = 0; i < m_components.size(); ++i) {
        int row = i / kColumns;
        int col = i % kColumns;
        m_grid->addWidget(m_components[i], row, col);
    }

    emit layoutChanged();
}

/**
 * @brief 从 QVariantMap 恢复布局（存根）
 * @param layout 布局描述
 */
void DashboardWidget::loadLayout(const QVariantMap &layout)
{
    Q_UNUSED(layout)
    /* TODO: 解析 layout 并重建子控件 */
}

/**
 * @brief 导出当前布局（存根）
 * @return 空的 QVariantMap
 */
QVariantMap DashboardWidget::saveLayout() const
{
    return QVariantMap();
}

/**
 * @brief 获取指定索引的子组件
 * @param index 索引
 * @return 子控件指针，越界返回 nullptr
 */
QWidget *DashboardWidget::componentAt(int index) const
{
    if (index < 0 || index >= m_components.size()) {
        return nullptr;
    }
    return m_components.at(index);
}

/**
 * @brief 获取子组件总数
 * @return 数量
 */
int DashboardWidget::componentCount() const
{
    return m_components.size();
}

/**
 * @brief 初始化 UI：创建网格布局并添加 6 个示例控件
 *
 * 示例布局（3列×2行）：
 * - Row 0: Gauge "电压" | Gauge "电流" | ProgressBar "功率"
 * - Row 1: LED "状态"   | Numeric "温度" | Numeric "转速"
 */
void DashboardWidget::setupUI()
{
    m_grid = new QGridLayout(this);
    m_grid->setSpacing(10);
    m_grid->setContentsMargins(10, 10, 10, 10);
    setLayout(m_grid);

    /* 示例控件 */
    addComponent("gauge",    tr("电压"));
    addComponent("gauge",    tr("电流"));
    addComponent("progress", tr("功率"));

    addComponent("led",     tr("状态"));
    addComponent("numeric", tr("温度"));
    addComponent("numeric", tr("转速"));
}
