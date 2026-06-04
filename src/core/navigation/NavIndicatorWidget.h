/** @file NavIndicatorWidget.h @brief 导航树选中滑动指示器 -- 左侧accent色竖线的平滑滑动动画(250ms OutCubic)。QPropertyAnimation驱动indicatorY, ThemeManager取色 */

#ifndef NAV_INDICATOR_WIDGET_H
#define NAV_INDICATOR_WIDGET_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTreeView>

/** @brief 导航树选中滑动指示器。透明覆盖层，在导航树左侧绘制3px宽accent色竖线，选中项切换时平滑滑动 */
class NavIndicatorWidget : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal indicatorY READ indicatorY WRITE setIndicatorY NOTIFY indicatorYChanged)

public:
    explicit NavIndicatorWidget(QTreeView* navTree, QWidget* parent = nullptr);
    void animateTo(const QModelIndex& index);              ///< 动画滑动(moveToIndex别名)
    void moveToIndex(const QModelIndex& index);            ///< 动画滑动到指定索引(250ms OutCubic)
    void jumpToIndex(const QModelIndex& index);            ///< 无动画跳转(用于初始化和恢复会话)
    qreal indicatorY() const;                              ///< 获取指示线Y坐标
    void setIndicatorY(qreal y);                           ///< QPropertyAnimation写访问器

    // ── 统计计数器 ──
    quint64 totalAnimations() const;       ///< 动画启动总次数
    quint64 totalPositionChanges() const;  ///< 位置变更总次数
    quint64 totalJumpMoves() const;        ///< 无动画跳转总次数
    quint64 totalThemeUpdates() const;     ///< 主题颜色更新总次数
    quint64 totalRepaints() const;         ///< 绘制事件总次数
    quint64 totalResizeSyncs() const;      ///< 尺寸同步总次数
    void resetIndicatorStatistics();       ///< 重置统计

signals:
    void indicatorYChanged(qreal y);       ///< 指示线Y坐标变化信号

public slots:
    void updateThemeColor();               ///< 主题切换时刷新颜色

protected:
    void paintEvent(QPaintEvent* event) override;  ///< 绘制指示线(左侧3px宽accent色竖线)
    bool eventFilter(QObject* watched, QEvent* event) override; ///< 监听navTree的resize

private:
    QTreeView* m_navTree;              ///< 关联的导航树控件
    QPropertyAnimation* m_slideAnim;   ///< 滑动动画（250ms OutCubic）
    qreal m_indicatorY;                ///< 指示线顶部 Y 坐标
    int m_indicatorHeight;             ///< 指示线高度（跟随选中项行高）

    // ── 统计计数器 ──
    quint64 m_totalAnimations = 0;     ///< 动画启动次数
    quint64 m_totalPositionChanges = 0;///< 位置变更次数
    quint64 m_totalJumpMoves = 0;      ///< 无动画跳转次数
    quint64 m_totalThemeUpdates = 0;   ///< 主题颜色更新次数
    quint64 m_totalRepaints = 0;       ///< 绘制事件次数
    quint64 m_totalResizeSyncs = 0;    ///< 尺寸同步次数
};

#endif // NAV_INDICATOR_WIDGET_H
