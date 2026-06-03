/**
 * @file ResponsiveLayout2.cpp
 * @brief 响应式布局v2实现 — 断点自适应网格布局
 */
#include "core/responsive/ResponsiveLayout2.h"
#include <QWidgetItem>

/** @brief 构造函数，初始化默认列数配置 @param parent 父Widget */
ResponsiveLayout::ResponsiveLayout(QWidget *parent) : QLayout(parent) { m_columns[Mobile] = 1; m_columns[Tablet] = 2; m_columns[Desktop] = 3; m_columns[Wide] = 4; }
/** @brief 析构函数，释放所有布局项 */
ResponsiveLayout::~ResponsiveLayout() { qDeleteAll(m_items); }
/** @brief 添加布局项 @param item 布局项指针 */
void ResponsiveLayout::addItem(QLayoutItem *item) { m_items.append(item); }
/** @brief 获取布局项数量 @return 布局项计数 */
int ResponsiveLayout::count() const { return m_items.size(); }
/** @brief 按索引获取布局项 @param i 索引 @return 布局项指针 */
QLayoutItem *ResponsiveLayout::itemAt(int i) const { return m_items.value(i); }
/** @brief 按索引移除并返回布局项 @param i 索引 @return 被移除的布局项指针 */
QLayoutItem *ResponsiveLayout::takeAt(int i) { return m_items.isEmpty() ? nullptr : m_items.takeAt(i); }
/** @brief 获取推荐尺寸 @return 最小尺寸 */
QSize ResponsiveLayout::sizeHint() const { return minimumSize(); }
/** @brief 获取最小尺寸 @return 所有子项最小宽度的最大值 */
QSize ResponsiveLayout::minimumSize() const { int w=0; for (auto *item : m_items) w = qMax(w, item->minimumSize().width()); return QSize(w, 0); }
/** @brief 设置布局边距 @param l 左 @param t 上 @param r 右 @param b 下 */
void ResponsiveLayout::setMargins(int l, int t, int r, int b) { setContentsMargins(l,t,r,b); }
/** @brief 设置布局间距 @param s 间距像素值 */
void ResponsiveLayout::setSpacing(int s) { QLayout::setSpacing(s); }
/** @brief 设置响应式断点宽度阈值 @param m 移动端断点 @param t 平板端断点 @param d 桌面端断点 */
void ResponsiveLayout::setBreakpoints(int m, int t, int d) { m_mobileBreak=m; m_tabletBreak=t; m_desktopBreak=d; }
/** @brief 根据当前宽度判断断点等级 @return 当前断点枚举值 */
ResponsiveLayout::Breakpoint ResponsiveLayout::currentBreakpoint() const { int w = geometry().width(); if (w < m_mobileBreak) return Mobile; if (w < m_tabletBreak) return Tablet; if (w < m_desktopBreak) return Desktop; return Wide; }
/** @brief 获取当前断点对应的列数 @return 列数 */
int ResponsiveLayout::columns() const { return m_columns.value(currentBreakpoint(), 1); }
/** @brief 设置指定断点的列数 @param bp 断点枚举 @param c 列数 */
void ResponsiveLayout::setColumnCount(Breakpoint bp, int c) { m_columns[bp] = c; }

/** @brief 执行布局 — 按当前断点列数均匀分配子项矩形区域 @param rect 可用布局区域 */
void ResponsiveLayout::setGeometry(const QRect &rect) {
    QLayout::setGeometry(rect);
    if (m_items.isEmpty()) return;
    int cols = columns();
    int spacing = this->spacing();
    QRect area = rect.adjusted(contentsMargins().left(), contentsMargins().top(), -contentsMargins().right(), -contentsMargins().bottom());
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
