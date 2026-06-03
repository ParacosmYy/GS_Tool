/**
 * @file ResponsiveLayout2.h
 * @brief 响应式网格布局管理器 - 根据容器宽度自动切换断点和列数
 *
 * 四断点体系（与 ResponsiveLayout 对齐）:
 *   - Mobile  (< 768px):   1 列
 *   - Tablet  (768~1023px): 2 列
 *   - Desktop (1024~1439px): 3 列
 *   - Wide    (>= 1440px):  4 列
 *
 * 职责:
 *   1. 根据容器宽度自动切换列数
 *   2. 均匀分配子项矩形区域
 *   3. 支持自定义各断点列数配置
 *   4. 支持 QSettings 持久化列数配置
 *   5. 统计断点切换次数
 */
#pragma once

#include <QLayout>
#include <QRect>
#include <QList>
#include <QWidgetItem>
#include <QMap>

/**
 * @class ResponsiveLayout
 * @brief 响应式网格布局，根据容器宽度在 Mobile/Tablet/Desktop/Wide 断点间自动切换列数
 */
class ResponsiveLayout : public QLayout {
    Q_OBJECT

public:
    /** @brief 响应式断点枚举（与 core/layout/ResponsiveLayout.h 对齐） */
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

    /** @brief 设置布局边距 @param left 左 @param top 上 @param right 右 @param bottom 下 */
    void setMargins(int left, int top, int right, int bottom);
    /** @brief 设置项间距 @param space 间距像素值 */
    void setSpacing(int space);

    /** @brief 获取当前生效的断点 @return 当前断点 */
    Breakpoint currentBreakpoint() const;
    /** @brief 获取当前断点对应的列数 @return 列数 */
    int columns() const;
    /** @brief 设置指定断点的列数 @param bp 断点 @param cols 列数 */
    void setColumnCount(Breakpoint bp, int cols);

    /** @brief 保存列数配置到 QSettings */
    void saveColumnConfig() const;
    /** @brief 从 QSettings 加载列数配置 */
    void loadColumnConfig();

    // ---- 统计 ----
    /** @brief 获取断点切换总次数 @return 切换次数 */
    quint64 breakpointChangeCount() const { return m_bpChangeCount; }
    /** @brief 重置统计计数器 */
    void resetStats() { m_bpChangeCount = 0; }

private:
    /** @brief 根据宽度计算断点（纯计算） @param w 宽度 @return 断点 */
    static Breakpoint calcBreakpoint(int w);

    QList<QLayoutItem *> m_items;        ///< 布局项列表
    int m_mobileBreak = 768;             ///< 移动端断点阈值(像素)
    int m_tabletBreak = 1024;            ///< 平板断点阈值(像素)
    int m_desktopBreak = 1440;           ///< 桌面断点阈值(像素)
    QMap<Breakpoint, int> m_columns;     ///< 各断点对应的列数配置
    Breakpoint m_lastBp = Mobile;        ///< 上次断点（用于检测变化）
    quint64 m_bpChangeCount = 0;         ///< 断点切换次数统计
};
