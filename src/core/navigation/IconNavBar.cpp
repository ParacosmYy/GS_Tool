/**
 * @file IconNavBar.cpp
 * @brief 图标导航栏实现 — 垂直图标栏用于三栏布局模式
 *
 * 56px宽垂直导航栏，显示分类图标，点击切换侧边栏面板列表。
 * 左侧绘制3px accent色激活指示线，颜色从ThemeManager获取。
 */

#include "core/navigation/IconNavBar.h"
#include "core/theme/ThemeManager.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QPainter>
#include <QPaintEvent>

// ─── 构造/析构 ───────────────────────────────────────────

IconNavBar::IconNavBar(QWidget* parent)
    : QWidget(parent)
{
    setObjectName("iconNavBar");
    setFixedWidth(navBarWidth());
    setAttribute(Qt::WA_StyledBackground, true);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(4, 8, 4, 8);
    m_layout->setSpacing(4);
    m_layout->setObjectName("iconNavBarLayout");
    m_layout->addStretch(); // 按钮插入到stretch之前

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);
}

// ─── 分类管理 ────────────────────────────────────────────

void IconNavBar::setCategories(const QVector<NavCategory>& categories)
{
    // 清除已有按钮
    qDeleteAll(m_buttons.values());
    m_buttons.clear();

    // 在stretch之前依次插入按钮
    for (int i = 0; i < categories.size(); ++i) {
        const auto& cat = categories[i];
        auto* btn = createCategoryButton(cat);
        m_layout->insertWidget(i, btn);
        m_buttons[cat.id] = btn;
        m_buttonGroup->addButton(btn, i);
    }

    m_activeIndex = -1;
    m_activeCategory.clear();
}

// ─── 激活状态 ────────────────────────────────────────────

void IconNavBar::setActiveCategory(const QString& id)
{
    if (m_activeCategory == id) return;

    m_activeCategory = id;
    ++m_totalCategorySwitches;

    // 更新按钮选中态和激活索引
    int index = 0;
    for (auto it = m_buttons.constBegin(); it != m_buttons.constEnd(); ++it, ++index) {
        if (it.key() == id) {
            it.value()->setChecked(true);
            m_activeIndex = index;
            break;
        }
    }

    update(); // 触发paintEvent重绘指示线
}

// ─── 按钮工厂 ────────────────────────────────────────────

QPushButton* IconNavBar::createCategoryButton(const NavCategory& category)
{
    auto* btn = new QPushButton(this);
    btn->setObjectName("iconNavBarButton_" + category.id);
    btn->setFixedSize(48, 48);
    btn->setToolTip(category.label);
    btn->setCheckable(true);

    // 占位图标文本: 取iconName首字符大写
    if (!category.iconName.isEmpty()) {
        btn->setText(QString(category.iconName.at(0)).toUpper());
    }

    connect(btn, &QPushButton::clicked, this, [this, id = category.id]() {
        ++m_totalButtonClicks;
        setActiveCategory(id);
        emit categoryClicked(id);
    });

    return btn;
}

// ─── 绘制 ────────────────────────────────────────────────

void IconNavBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    // 无激活项时不绘制
    if (m_activeIndex < 0) return;

    // 查找激活按钮的几何位置
    QWidget* activeBtn = nullptr;
    int idx = 0;
    for (auto it = m_buttons.constBegin(); it != m_buttons.constEnd(); ++it, ++idx) {
        if (idx == m_activeIndex) {
            activeBtn = it.value();
            break;
        }
    }
    if (!activeBtn) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 从ThemeManager获取Accent颜色（禁止硬编码）
    const QColor accentColor =
        ThemeManager::instance().color(ThemeManager::SemanticColor::Accent);

    // 指示线参数: 宽3px，左侧对齐，圆角2px
    constexpr int kLineWidth = 3;
    constexpr int kRadius = 2;

    // 按钮在IconNavBar坐标系中的位置
    const int btnY = activeBtn->y();
    const int btnH = activeBtn->height();

    QRectF indicatorRect(0, btnY, kLineWidth, btnH);

    painter.setPen(Qt::NoPen);
    painter.setBrush(accentColor);
    painter.drawRoundedRect(indicatorRect, kRadius, kRadius);
}

// ─── 统计重置 ────────────────────────────────────────────

void IconNavBar::resetNavStatistics()
{
    m_totalCategorySwitches = 0;
    m_totalButtonClicks = 0;
}
