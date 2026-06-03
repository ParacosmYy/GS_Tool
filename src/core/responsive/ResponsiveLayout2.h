/**
 * @file ResponsiveLayout2.h
 * @brief 响应式布局管理器，根据容器宽度自动切换断点和列数
 */
#pragma once
#include <QLayout>
#include <QRect>
#include <QList>
#include <QWidgetItem>

/**
 * @class ResponsiveLayout
 * @brief 响应式布局，根据容器宽度在Mobile/Tablet/Desktop/Wide断点间自动切换列数和布局策略
 */
class ResponsiveLayout : public QLayout {
    Q_OBJECT
public:
    /** @brief 响应式断点枚举 */
    enum Breakpoint { Mobile, Tablet, Desktop, Wide };
    Q_ENUM(Breakpoint)

    /** @brief 构造函数 @param parent 父控件指针 */
    explicit ResponsiveLayout(QWidget *parent = nullptr);
    /** @brief 析构函数 */
    ~ResponsiveLayout() override;

    /** @brief 添加布局项 @param item 布局项 */
    void addItem(QLayoutItem *item) override;
    /** @brief 获取布局项数量 @return 项数量 */
    int count() const override;
    /** @brief 按索引获取布局项 @param index 索引 @return 布局项指针 */
    QLayoutItem *itemAt(int index) const override;
    /** @brief 移除并返回指定索引的布局项 @param index 索引 @return 被移除的布局项 */
    QLayoutItem *takeAt(int index) override;
    /** @brief 返回推荐大小 @return 推荐尺寸 */
    QSize sizeHint() const override;
    /** @brief 返回最小大小 @return 最小尺寸 */
    QSize minimumSize() const override;
    /** @brief 设置布局几何区域并执行排列 @param rect 布局矩形 */
    void setGeometry(const QRect &rect) override;
    /** @brief 设置布局边距 @param left 左边距 @param top 上边距 @param right 右边距 @param bottom 下边距 */
    void setMargins(int left, int top, int right, int bottom);
    /** @brief 设置项间距 @param space 间距像素值 */
    void setSpacing(int space);
    /** @brief 设置断点像素阈值 @param mobile 移动端阈值 @param tablet 平板阈值 @param desktop 桌面阈值 */
    void setBreakpoints(int mobile, int tablet, int desktop);
    /** @brief 获取当前生效的断点 @return 当前断点 */
    Breakpoint currentBreakpoint() const;
    /** @brief 获取当前断点对应的列数 @return 列数 */
    int columns() const;
    /** @brief 设置指定断点的列数 @param bp 断点 @param cols 列数 */
    void setColumnCount(Breakpoint bp, int cols);

private:
    QList<QLayoutItem *> m_items;        ///< 布局项列表
    int m_mobileBreak = 600;             ///< 移动端断点阈值(像素)
    int m_tabletBreak = 900;             ///< 平板断点阈值(像素)
    int m_desktopBreak = 1200;           ///< 桌面断点阈值(像素)
    QMap<Breakpoint, int> m_columns;     ///< 各断点对应的列数配置
};
