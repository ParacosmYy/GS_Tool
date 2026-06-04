/**
 * @file DashboardWidgetLayout.cpp
 * @brief 仪表盘主容器 — 布局管理方法实现
 *
 * 从 DashboardWidget.cpp 拆分而来，包含:
 *   - loadLayout(): 从QVariantMap恢复布局
 *   - saveLayout(): 导出到QVariantMap
 *   - setupUI(): 初始化网格布局和示例控件
 *
 * 组件增删和属性序列化见 DashboardWidget.cpp。
 * 持久化方法见 DashboardWidgetPersistence.cpp。
 * 事件处理器与统计接口见 DashboardWidgetSlots.cpp。
 */

#include "dashboard/DashboardWidget.h"
#include "dashboard/GaugeWidget.h"
#include "dashboard/ProgressBarWidget.h"
#include "dashboard/LedIndicatorWidget.h"
#include "dashboard/NumericDisplayWidget.h"
#include "core/theme/ThemeManager.h"

/** @brief 从QVariantMap恢复布局，清除现有子控件后按列表顺序重建 @param layout 布局描述，格式为{"components":[{"type":"gauge","channel":"电压"},...]} */
void DashboardWidget::loadLayout(const QVariantMap &layout)
{
    /* 清除现有组件 */
    while (!m_components.isEmpty()) {
        QWidget *w = m_components.takeLast();
        m_grid->removeWidget(w);
        delete w;
    }
    m_totalLayoutChanges = 0;

    /* 从布局数据重建 */
    const QVariantList comps = layout.value("components").toList();
    for (const QVariant& cv : comps) {
        const QVariantMap cm = cv.toMap();
        const QString type = cm.value("type").toString();
        const QString channel = cm.value("channel").toString();
        if (!type.isEmpty()) {
            addComponent(type, channel);
        }
    }
}

/** @brief 将当前布局导出为QVariantMap，序列化每个子组件的类型和绑定通道名 @return 布局描述，格式同loadLayout */
QVariantMap DashboardWidget::saveLayout() const
{
    QVariantMap result;
    QVariantList comps;

    for (int i = 0; i < m_components.size(); ++i) {
        QWidget* w = m_components.at(i);
        QVariantMap cm;

        /* 根据控件类型名推断组件类型 */
        QString className = w->metaObject()->className();
        if (className.contains("Gauge")) {
            cm["type"] = "gauge";
        } else if (className.contains("ProgressBar")) {
            cm["type"] = "progress";
        } else if (className.contains("LedIndicator")) {
            cm["type"] = "led";
        } else if (className.contains("NumericDisplay")) {
            cm["type"] = "numeric";
        } else {
            cm["type"] = "unknown";
        }

        /* 读取绑定的通道名 (通过 property) */
        cm["channel"] = w->property("channel").toString();

        comps.append(cm);
    }

    result["components"] = comps;
    return result;
}

/** @brief 初始化UI，创建网格布局并添加6个示例控件（3列x2行: Gauge电压/Gauge电流/ProgressBar功率, LED状态/Numeric温度/Numeric转速） */
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
