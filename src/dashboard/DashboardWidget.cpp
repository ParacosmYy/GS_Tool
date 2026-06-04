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


// loadLayout/saveLayout/setupUI见 DashboardWidgetLayout.cpp
// 完整属性序列化见 DashboardWidgetSerialization.cpp
// 持久化方法(serializer/saveToFile/loadFromFile/saveToProfile/loadFromProfile)见 DashboardWidgetPersistence.cpp
// 事件处理器与统计接口见 DashboardWidgetSlots.cpp
