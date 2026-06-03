#pragma once
#include <QLayout>
#include <QRect>
#include <QList>
#include <QWidgetItem>

class ResponsiveLayout : public QLayout {
    Q_OBJECT
public:
    enum Breakpoint { Mobile, Tablet, Desktop, Wide };
    Q_ENUM(Breakpoint)

    explicit ResponsiveLayout(QWidget *parent = nullptr);
    ~ResponsiveLayout() override;
    void addItem(QLayoutItem *item) override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    void setMargins(int left, int top, int right, int bottom);
    void setSpacing(int space);
    void setBreakpoints(int mobile, int tablet, int desktop);
    Breakpoint currentBreakpoint() const;
    int columns() const;
    void setColumnCount(Breakpoint bp, int cols);
private:
    QList<QLayoutItem *> m_items;
    int m_mobileBreak = 600;
    int m_tabletBreak = 900;
    int m_desktopBreak = 1200;
    QMap<Breakpoint, int> m_columns;
};
