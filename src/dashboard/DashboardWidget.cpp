/**
 * @file DashboardWidget.cpp
 * @brief 仪表盘主容器控件实现
 *
 * 以 QGridLayout(3列) 管理仪表盘子组件。
 * 构造时添加 6 个示例控件（2xGauge、1xProgressBar、1xLED、2xNumericDisplay），
 * 支持动态增删与完整属性序列化。
 *
 * 完整属性序列化:
 * - gauge: min/max范围、label标签、channel通道、value当前值
 * - progress: min/max范围、label标签、channel通道、value当前值
 * - led: on/off状态、color颜色、channel通道
 * - numeric: unit单位、precision精度、channel通道、value当前值
 */

#include "dashboard/DashboardWidget.h"
#include "dashboard/GaugeWidget.h"
#include "dashboard/ProgressBarWidget.h"
#include "dashboard/LedIndicatorWidget.h"
#include "dashboard/NumericDisplayWidget.h"
#include "core/theme/ThemeManager.h"

/** @brief 构造函数，初始化UI、序列化器与示例控件 @param parent 父控件 */
DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent)
    , m_grid(nullptr)
    , m_serializer(new DashboardSerializer(this))
{
    setObjectName("DashboardWidget");
    setupUI();
}

/** @brief 析构函数，子控件由Qt父子树自动销毁 */
DashboardWidget::~DashboardWidget() = default;

/** @brief 添加一个仪表盘子组件 @param type 组件类型: "gauge"/"progress"/"led"/"numeric" @param channel 绑定数据通道 @return 组件索引，失败返回-1 */
int DashboardWidget::addComponent(const QString &type, const QString &channel)
{
    QWidget *widget = nullptr;

    if (type == QLatin1String("gauge")) {
        auto *g = new GaugeWidget(this);
        g->setLabel(channel.isEmpty() ? tr("未命名") : channel);
        g->bindChannel(channel);
        g->setRange(0.0, 100.0);
        ++m_totalRangeChanges;
        g->setValue(42.0);
        ++m_totalUpdates;
        ++m_totalValueChanged;
        widget = g;
    } else if (type == QLatin1String("progress")) {
        auto *p = new ProgressBarWidget(this);
        p->setLabel(channel.isEmpty() ? tr("进度") : channel);
        p->bindChannel(channel);
        p->setRange(0.0, 100.0);
        ++m_totalRangeChanges;
        p->setValue(65.0);
        ++m_totalUpdates;
        ++m_totalValueChanged;
        widget = p;
    } else if (type == QLatin1String("led")) {
        auto *led = new LedIndicatorWidget(this);
        led->setOn(true);
        led->setColor(ThemeManager::instance().color(ThemeManager::SemanticColor::Success));
        led->bindChannel(channel);
        ++m_totalUpdates;
        ++m_totalValueChanged;
        widget = led;
    } else if (type == QLatin1String("numeric")) {
        auto *n = new NumericDisplayWidget(this);
        n->bindChannel(channel);
        n->setUnit(tr("V"));
        n->setPrecision(2);
        n->setValue(3.30);
        ++m_totalUpdates;
        ++m_totalValueChanged;
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

    ++m_totalWidgetsAdded;
    ++m_totalLayoutChanges;
    emit layoutChanged();
    return index;
}

/** @brief 移除指定索引的组件并重新排列网格 @param index 组件索引 */
void DashboardWidget::removeComponent(int index)
{
    if (index < 0 || index >= m_components.size()) {
        return;
    }

    QWidget *w = m_components.takeAt(index);
    m_grid->removeWidget(w);
    delete w;

    ++m_totalWidgetsRemoved;
    ++m_totalLayoutChanges;

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

// 完整属性序列化见 DashboardWidgetSerialization.cpp
// 持久化方法(serializer/saveToFile/loadFromFile/saveToProfile/loadFromProfile)见 DashboardWidgetPersistence.cpp

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

// 事件处理器与统计接口见 DashboardWidgetSlots.cpp
