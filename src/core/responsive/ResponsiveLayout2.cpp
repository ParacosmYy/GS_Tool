/**
 * @file ResponsiveLayout2.cpp
 * @brief 响应式网格布局实现 — 四断点自适应列数布局
 *
 * 四断点体系:
 *   Mobile  (< 768px):   默认1列
 *   Tablet  (768~1023px): 默认2列
 *   Desktop (1024~1439px): 默认3列
 *   Wide    (>= 1440px):  默认4列
 *
 * 布局算法: 按当前断点列数均匀分配子项到网格单元格。
 */
#include "core/responsive/ResponsiveLayout2.h"
#include <QSettings>

/** @brief 构造函数，初始化默认列数配置 @param parent 父Widget */
ResponsiveLayout::ResponsiveLayout(QWidget *parent)
    : QLayout(parent)
{
    m_columns[Mobile]  = 1;
    m_columns[Tablet]  = 2;
    m_columns[Desktop] = 3;
    m_columns[Wide]    = 4;
    loadColumnConfig();
}

/** @brief 析构函数，释放所有布局项 */
ResponsiveLayout::~ResponsiveLayout()
{
    qDeleteAll(m_items);
}

/** @brief 添加布局项 @param item 布局项指针 */
void ResponsiveLayout::addItem(QLayoutItem *item)
{
    m_items.append(item);
}

/** @brief 获取布局项数量 @return 布局项计数 */
int ResponsiveLayout::count() const
{
    return m_items.size();
}

/** @brief 按索引获取布局项 @param i 索引 @return 布局项指针 */
QLayoutItem *ResponsiveLayout::itemAt(int i) const
{
    return m_items.value(i);
}

/** @brief 按索引移除并返回布局项 @param i 索引 @return 被移除的布局项指针 */
QLayoutItem *ResponsiveLayout::takeAt(int i)
{
    return m_items.isEmpty() ? nullptr : m_items.takeAt(i);
}

/** @brief 获取推荐尺寸 @return 最小尺寸 */
QSize ResponsiveLayout::sizeHint() const
{
    return minimumSize();
}

/** @brief 获取最小尺寸 @return 所有子项最小宽度的最大值 */
QSize ResponsiveLayout::minimumSize() const
{
    int w = 0;
    for (auto *item : m_items)
        w = qMax(w, item->minimumSize().width());
    return QSize(w, 0);
}

/** @brief 设置布局边距 @param l 左 @param t 上 @param r 右 @param b 下 */
void ResponsiveLayout::setMargins(int l, int t, int r, int b)
{
    setContentsMargins(l, t, r, b);
}

/** @brief 设置布局间距 @param s 间距像素值 */
void ResponsiveLayout::setSpacing(int s)
{
    QLayout::setSpacing(s);
}

/** @brief 根据宽度计算断点（纯计算） @param w 宽度 @return 断点枚举 */
ResponsiveLayout::Breakpoint ResponsiveLayout::calcBreakpoint(int w)
{
    if (w < 768)  return Mobile;
    if (w < 1024) return Tablet;
    if (w < 1440) return Desktop;
    return Wide;
}

/** @brief 根据当前宽度判断断点等级 @return 当前断点枚举值 */
ResponsiveLayout::Breakpoint ResponsiveLayout::currentBreakpoint() const
{
    return calcBreakpoint(geometry().width());
}

/** @brief 获取当前断点对应的列数 @return 列数 */
int ResponsiveLayout::columns() const
{
    return m_columns.value(currentBreakpoint(), 1);
}

/** @brief 设置指定断点的列数 @param bp 断点枚举 @param c 列数 */
void ResponsiveLayout::setColumnCount(Breakpoint bp, int c)
{
    ++m_totalColumnChanges;
    m_columns[bp] = c;
}

/** @brief 保存列数配置到 QSettings */
void ResponsiveLayout::saveColumnConfig() const
{
    ++m_totalConfigSaves;
    QSettings settings;
    settings.beginGroup("layout/responsiveGrid");
    settings.setValue("colsMobile",  m_columns.value(Mobile,  1));
    settings.setValue("colsTablet",  m_columns.value(Tablet,  2));
    settings.setValue("colsDesktop", m_columns.value(Desktop, 3));
    settings.setValue("colsWide",    m_columns.value(Wide,    4));
    settings.endGroup();
}

/** @brief 从 QSettings 加载列数配置 */
void ResponsiveLayout::loadColumnConfig()
{
    ++m_totalConfigLoads;
    QSettings settings;
    settings.beginGroup("layout/responsiveGrid");
    if (settings.contains("colsMobile"))
        m_columns[Mobile] = settings.value("colsMobile", 1).toInt();
    if (settings.contains("colsTablet"))
        m_columns[Tablet] = settings.value("colsTablet", 2).toInt();
    if (settings.contains("colsDesktop"))
        m_columns[Desktop] = settings.value("colsDesktop", 3).toInt();
    if (settings.contains("colsWide"))
        m_columns[Wide] = settings.value("colsWide", 4).toInt();
    settings.endGroup();
}

/**
 * @brief 执行布局 — 按当前断点列数均匀分配子项矩形区域
 *
 * 检测断点变化并递增统计计数器。
 * 按列数计算每个单元格的宽高，依次排列子项。
 *
 * @param rect 可用布局区域
 */
void ResponsiveLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    ++m_totalLayoutUpdates;
    if (m_items.isEmpty()) return;

    // 检测断点变化
    Breakpoint bp = calcBreakpoint(rect.width());
    if (bp != m_lastBp) {
        ++m_bpChangeCount;
        m_lastBp = bp;
    }

    int cols = m_columns.value(bp, 1);
    int spacing = this->spacing();
    QRect area = rect.adjusted(contentsMargins().left(), contentsMargins().top(),
                               -contentsMargins().right(), -contentsMargins().bottom());
    int cellW = (area.width() - (cols - 1) * spacing) / cols;
    int rows = (m_items.size() + cols - 1) / cols;
    int cellH = rows > 0 ? (area.height() - (rows - 1) * spacing) / rows : area.height();

    for (int i = 0; i < m_items.size(); ++i) {
        int row = i / cols, col = i % cols;
        int x = area.x() + col * (cellW + spacing);
        int y = area.y() + row * (cellH + spacing);
        m_items[i]->setGeometry(QRect(x, y, cellW, cellH));
    }
}

/** @brief 重置所有统计计数器 */
void ResponsiveLayout::resetStats() {
    m_bpChangeCount = 0; m_totalLayoutUpdates = 0; m_totalColumnChanges = 0;
    m_totalConfigSaves = 0; m_totalConfigLoads = 0;
}
