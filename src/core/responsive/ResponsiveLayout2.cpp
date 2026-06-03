#include "core/responsive/ResponsiveLayout2.h"
#include <QWidgetItem>
ResponsiveLayout::ResponsiveLayout(QWidget *parent) : QLayout(parent) { m_columns[Mobile] = 1; m_columns[Tablet] = 2; m_columns[Desktop] = 3; m_columns[Wide] = 4; }
ResponsiveLayout::~ResponsiveLayout() { qDeleteAll(m_items); }
void ResponsiveLayout::addItem(QLayoutItem *item) { m_items.append(item); }
int ResponsiveLayout::count() const { return m_items.size(); }
QLayoutItem *ResponsiveLayout::itemAt(int i) const { return m_items.value(i); }
QLayoutItem *ResponsiveLayout::takeAt(int i) { return m_items.isEmpty() ? nullptr : m_items.takeAt(i); }
QSize ResponsiveLayout::sizeHint() const { return minimumSize(); }
QSize ResponsiveLayout::minimumSize() const { int w=0; for (auto *item : m_items) w = qMax(w, item->minimumSize().width()); return QSize(w, 0); }
void ResponsiveLayout::setMargins(int l, int t, int r, int b) { setContentsMargins(l,t,r,b); }
void ResponsiveLayout::setSpacing(int s) { QLayout::setSpacing(s); }
void ResponsiveLayout::setBreakpoints(int m, int t, int d) { m_mobileBreak=m; m_tabletBreak=t; m_desktopBreak=d; }
ResponsiveLayout::Breakpoint ResponsiveLayout::currentBreakpoint() const { int w = geometry().width(); if (w < m_mobileBreak) return Mobile; if (w < m_tabletBreak) return Tablet; if (w < m_desktopBreak) return Desktop; return Wide; }
int ResponsiveLayout::columns() const { return m_columns.value(currentBreakpoint(), 1); }
void ResponsiveLayout::setColumnCount(Breakpoint bp, int c) { m_columns[bp] = c; }

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
