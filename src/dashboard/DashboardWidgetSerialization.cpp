/**
 * @file DashboardWidgetSerialization.cpp
 * @brief 仪表盘面板完整属性序列化/反序列化
 *
 * 从DashboardWidget.cpp拆分，负责:
 *   1. extractWidgetConfig: 从QWidget提取完整属性到DashboardItemConfig
 *   2. createWidgetFromConfig: 根据DashboardItemConfig创建并配置组件
 *   3. saveToItems: 将当前布局导出为DashboardItemConfig列表
 *   4. loadFromItems: 从DashboardItemConfig列表恢复布局
 *
 * 支持gauge/progressbar/led/numeric四种面板类型的完整属性持久化。
 */

#include "dashboard/DashboardWidget.h"
#include "dashboard/GaugeWidget.h"
#include "dashboard/ProgressBarWidget.h"
#include "dashboard/LedIndicatorWidget.h"
#include "dashboard/NumericDisplayWidget.h"

#include <QColor>
#include <QLatin1String>

/** @brief 从单个QWidget提取完整属性到DashboardItemConfig @param widget 目标控件 @param index 组件索引（用于计算网格位置） @return 完整的面板配置项 */
DashboardItemConfig DashboardWidget::extractWidgetConfig(QWidget *widget, int index) const
{
    DashboardItemConfig config;
    config.row = index / kColumns;
    config.column = index % kColumns;

    QString className = widget->metaObject()->className();

    if (className.contains("Gauge")) {
        auto *g = qobject_cast<GaugeWidget*>(widget);
        config.widgetType = QStringLiteral("gauge");
        config.title = g->label();
        config.properties[QStringLiteral("channel")] = g->channelName();
        config.properties[QStringLiteral("value")] = g->value();
        config.properties[QStringLiteral("min")] = g->min();
        config.properties[QStringLiteral("max")] = g->max();
    } else if (className.contains("ProgressBar")) {
        auto *p = qobject_cast<ProgressBarWidget*>(widget);
        config.widgetType = QStringLiteral("progressbar");
        config.title = p->label();
        config.properties[QStringLiteral("channel")] = p->channelName();
        config.properties[QStringLiteral("value")] = p->value();
        config.properties[QStringLiteral("min")] = p->min();
        config.properties[QStringLiteral("max")] = p->max();
    } else if (className.contains("LedIndicator")) {
        auto *led = qobject_cast<LedIndicatorWidget*>(widget);
        config.widgetType = QStringLiteral("led");
        config.title = led->channelName();
        config.properties[QStringLiteral("channel")] = led->channelName();
        config.properties[QStringLiteral("on")] = led->isOn();
        config.properties[QStringLiteral("color")] = led->color().name(QColor::HexArgb);
    } else if (className.contains("NumericDisplay")) {
        auto *n = qobject_cast<NumericDisplayWidget*>(widget);
        config.widgetType = QStringLiteral("numeric");
        config.title = n->channelName();
        config.properties[QStringLiteral("channel")] = n->channelName();
        config.properties[QStringLiteral("value")] = n->value();
        config.properties[QStringLiteral("unit")] = n->unit();
        config.properties[QStringLiteral("precision")] = n->precision();
    } else {
        config.widgetType = QStringLiteral("unknown");
    }

    return config;
}

/** @brief 根据DashboardItemConfig创建并配置单个组件 @param config 面板配置项 @return 创建的控件指针，失败返回nullptr */
QWidget* DashboardWidget::createWidgetFromConfig(const DashboardItemConfig &config)
{
    const QString& type = config.widgetType;
    const QMap<QString, QVariant>& props = config.properties;

    if (type == QLatin1String("gauge")) {
        auto *g = new GaugeWidget(this);
        g->setLabel(config.title);
        g->bindChannel(props.value("channel").toString());
        g->setRange(props.value("min", 0.0).toDouble(),
                    props.value("max", 100.0).toDouble());
        g->setValue(props.value("value", 0.0).toDouble());
        return g;
    }

    if (type == QLatin1String("progressbar")) {
        auto *p = new ProgressBarWidget(this);
        p->setLabel(config.title);
        p->bindChannel(props.value("channel").toString());
        p->setRange(props.value("min", 0.0).toDouble(),
                    props.value("max", 100.0).toDouble());
        p->setValue(props.value("value", 0.0).toDouble());
        return p;
    }

    if (type == QLatin1String("led")) {
        auto *led = new LedIndicatorWidget(this);
        led->bindChannel(props.value("channel").toString());
        led->setOn(props.value("on").toBool());
        const QString colorName = props.value("color").toString();
        if (!colorName.isEmpty()) {
            led->setColor(QColor(colorName));
        }
        return led;
    }

    if (type == QLatin1String("numeric")) {
        auto *n = new NumericDisplayWidget(this);
        n->bindChannel(props.value("channel").toString());
        n->setUnit(props.value("unit").toString());
        n->setPrecision(props.value("precision", 2).toInt());
        n->setValue(props.value("value", 0.0).toDouble());
        return n;
    }

    return nullptr;
}

/** @brief 将当前布局导出为DashboardItemConfig列表（完整属性序列化） @return 面板配置列表 */
QList<DashboardItemConfig> DashboardWidget::saveToItems() const
{
    QList<DashboardItemConfig> items;
    items.reserve(m_components.size());

    for (int i = 0; i < m_components.size(); ++i) {
        items.append(extractWidgetConfig(m_components.at(i), i));
    }

    return items;
}

/** @brief 从DashboardItemConfig列表恢复布局（完整属性反序列化） @param items 面板配置列表 @param columns 网格列数（用于验证，当前未使用） */
void DashboardWidget::loadFromItems(const QList<DashboardItemConfig> &items, int columns)
{
    Q_UNUSED(columns);

    /* 清除现有组件 */
    while (!m_components.isEmpty()) {
        QWidget *w = m_components.takeLast();
        m_grid->removeWidget(w);
        delete w;
    }

    /* 从完整配置重建 */
    for (const DashboardItemConfig &config : items) {
        QWidget *widget = createWidgetFromConfig(config);
        if (!widget) {
            continue;
        }

        int index = m_components.size();
        int row = config.row;
        int col = config.column;

        /* 如果配置中有合法位置，使用配置位置；否则自动排列 */
        if (row < 0 || col < 0 || col >= kColumns) {
            row = index / kColumns;
            col = index % kColumns;
        }

        m_grid->addWidget(widget, row, col,
                          config.rowSpan, config.columnSpan);
        m_components.append(widget);

        ++m_totalWidgetsAdded;
    }

    ++m_totalLayoutChanges;
    ++m_totalFullLoads;
    emit layoutChanged();
}
