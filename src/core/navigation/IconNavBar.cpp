/**
 * @file IconNavBar.cpp
 * @brief 图标导航栏实现 — 垂直图标栏用于三栏布局模式
 *
 * 56px宽垂直导航栏，显示分类图标，点击切换侧边栏面板列表。
 * 左侧绘制3px accent色激活指示线，颜色从ThemeManager获取。
 */

#include "core/navigation/IconNavBar.h"
#include "core/navigation/NavigationController.h"
#include "core/theme/IconManager.h"
#include "core/theme/ThemeManager.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QPainter>
#include <QPaintEvent>
#include <QCoreApplication>
#include <QSet>

// ─── 构造/析构 ───────────────────────────────────────────

/** @brief 构造函数，初始化导航栏布局、按钮组和固定宽度 @param parent 父控件指针 */
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

/** @brief 设置导航栏分类列表，清除已有按钮并重建 @param categories 分类信息向量(NavCategory) */
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

/** @brief 从面板映射生成分类列表，保持映射中分类首次出现顺序 */
QVector<NavCategory> IconNavBar::categoriesFromMappings(const QVector<NavPanelMapping>& mappings)
{
    QVector<NavCategory> categories;
    QSet<QString> seen;

    for (const auto& mapping : mappings) {
        const QString categoryId = QString::fromUtf8(mapping.category);
        if (!seen.contains(categoryId)) {
            NavCategory category;
            category.id = categoryId;
            category.label = QCoreApplication::translate("Nav", mapping.category);
            category.iconName = QString::fromUtf8(mapping.iconName);
            category.panelIds.append(QString::fromUtf8(mapping.id));
            categories.append(category);
            seen.insert(categoryId);
            continue;
        }

        for (auto& category : categories) {
            if (category.id == categoryId) {
                category.panelIds.append(QString::fromUtf8(mapping.id));
                break;
            }
        }
    }

    return categories;
}

// ─── 激活状态 ────────────────────────────────────────────

/** @brief 设置当前激活的分类，更新按钮选中态并触发重绘 @param id 分类标识符 */
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

/** @brief 创建分类导航按钮，设置固定尺寸和点击信号 @param category 分类信息 @return 新创建的QPushButton指针 */
QPushButton* IconNavBar::createCategoryButton(const NavCategory& category)
{
    auto* btn = new QPushButton(this);
    btn->setObjectName("iconNavBarButton_" + category.id);
    btn->setFixedSize(48, 48);
    QString tooltip = tr("%1 · %2 个面板").arg(category.label).arg(category.panelIds.size());
    if (category.panelIds.size() > 1) {
        tooltip += QLatin1Char('\n') + tr("再次点击切换下一个");
    }
    btn->setToolTip(tooltip);
    btn->setCheckable(true);

    // 优先使用统一 IconManager，资源缺失时才退回文本占位。
    if (!category.iconName.isEmpty()) {
        const QIcon icon = IconManager::instance().icon(category.iconName);
        if (!icon.isNull()) {
            btn->setIcon(icon);
            btn->setIconSize(QSize(20, 20));
            btn->setText(QString());
        } else {
            btn->setText(QString(category.iconName.at(0)).toUpper());
        }
    }

    connect(btn, &QPushButton::clicked, this, [this, id = category.id]() {
        ++m_totalButtonClicks;
        setActiveCategory(id);
        emit categoryClicked(id);
    });

    return btn;
}

// ─── 绘制 ────────────────────────────────────────────────

/** @brief 绘制事件处理，在激活按钮左侧绘制3px圆角accent色指示线 @param event 绘制事件(未使用) */
void IconNavBar::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    ++m_totalRenders;

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

/** @brief 重置导航栏所有统计计数器 */
void IconNavBar::resetNavStatistics()
{
    m_totalCategorySwitches = 0;
    m_totalButtonClicks = 0;
    m_totalRenders = 0;
}
