/**
 * @file BasePanel.h
 * @brief BasePanel容器组件 — 为所有功能面板提供统一的标题栏、折叠和动画
 *
 * BasePanel包裹一个内容QWidget，在顶部添加统一的标题栏:
 *   [icon(16px)] [标题文字(bold 13px)] [弹性空间] [折叠按钮(chevron)]
 * 面板显示/隐藏时带有淡入+滑入/淡出+滑出动画。
 * 设计模式: 装饰器(Decorator) — 包装现有QWidget，添加标题栏和动画能力
 * 协作: PanelManager通过m_wrappers映射管理BasePanel实例
 */
#ifndef BASE_PANEL_H
#define BASE_PANEL_H

#include <QWidget>
#include <QPointer>

class QLabel;
class QPushButton;
class QGraphicsOpacityEffect;
class EmptyStateWidget;
class LoadingSpinner;

/** @brief BasePanel容器 — 统一面板视觉外壳(纯表现层组件，不包含业务逻辑) */
class BasePanel : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal panelOpacity READ panelOpacity WRITE setPanelOpacity)

public:
    /** @brief 构造BasePanel容器 @param content 被包裹的内容面板 @param title 标题栏文字(必须tr()) @param parent 父控件 */
    explicit BasePanel(QWidget* content, const QString& title, QWidget* parent = nullptr);
    QWidget* contentWidget() const;             ///< 获取被包裹的原始内容面板
    void setTitle(const QString& title);        ///< 设置标题栏标题
    QString title() const;                      ///< 获取标题栏标题
    /** @brief 设置标题栏图标名称(Lucide图标名) @param name 图标名称如"cable" */
    void setIconName(const QString& name);
    void setCollapsible(bool enabled);          ///< 设置是否可折叠
    bool isCollapsible() const;                 ///< 获取是否可折叠
    void setCollapsed(bool collapsed);          ///< 折叠/展开内容区域
    bool isCollapsed() const;                   ///< 获取折叠状态
    qreal panelOpacity() const;                 ///< 动画透明度属性
    void setPanelOpacity(qreal opacity);        ///< 设置动画透明度属性
    void showEmptyState(const QString& title, const QString& description = QString()); ///< 显示空状态占位
    void hideEmptyState();                      ///< 隐藏空状态占位
    void showLoading();                         ///< 显示加载旋转指示器
    void hideLoading();                         ///< 隐藏加载指示器
    void showSkeleton();                        ///< 显示骨架屏占位
    void hideSkeleton();                        ///< 隐藏骨架屏占位

public slots:
    void animateShow();  ///< 带动画显示面板(opacity 0→1, slide Y:-20→0, 250ms OutCubic)
    void animateHide();  ///< 带动画隐藏面板(opacity 1→0, slide Y:0→-20, 200ms InCubic)

signals:
    void collapsedChanged(bool collapsed); ///< 折叠状态变更信号

protected:
    void paintEvent(QPaintEvent* event) override; ///< 绘制微阴影效果

private:
    void setupInternalLayout();   ///< 构建内部布局(标题栏+内容区域)
    void updateCollapseIcon();    ///< 更新折叠按钮图标方向
    void toggleCollapsed();       ///< 切换折叠状态
    bool eventFilter(QObject* watched, QEvent* event) override; ///< 标题栏事件过滤器

protected:
    QWidget*     m_content;       ///< 被包裹的内容面板(不拥有)
    QLabel*      m_iconLabel;     ///< objectName="panelIcon"
    QLabel*      m_titleLabel;    ///< objectName="panelTitle"
    QPushButton* m_collapseBtn;   ///< objectName="collapseButton"
    QWidget*     m_headerBar;     ///< objectName="panelHeaderBar"
    QWidget*     m_contentArea;   ///< objectName="panelContentArea"
    EmptyStateWidget* m_emptyState = nullptr;     ///< objectName="panelEmptyState"
    LoadingSpinner*   m_loadingSpinner = nullptr;  ///< objectName="panelLoadingSpinner"
    QWidget*          m_skeleton = nullptr;          ///< objectName="panelSkeleton"
    bool m_collapsible = true, m_collapsed = false;
    QString m_iconName;           ///< 图标名称
    QPointer<QGraphicsOpacityEffect> m_opacityEffect; ///< 透明度特效
    qreal m_panelOpacity = 1.0;   ///< 动画用透明度属性
    // ---- 统计计数器 ----
    quint64 m_totalToggles = 0, m_totalExpansions = 0, m_totalCollapses = 0;
    quint64 m_totalShows = 0, m_totalHides = 0, m_totalTitleChanges = 0;
    quint64 m_totalTitleClicks = 0, m_totalDragStarts = 0, m_totalSettingsOpens = 0;
    quint64 m_totalEmptyStateShows = 0, m_totalLoadingShows = 0, m_totalSkeletonShows = 0;

public:
    quint64 totalToggles() const { return m_totalToggles; }       ///< 获取总折叠切换次数
    quint64 totalExpansions() const { return m_totalExpansions; }  ///< 获取总展开次数
    quint64 totalCollapses() const { return m_totalCollapses; }    ///< 获取总折叠次数
    quint64 totalShows() const { return m_totalShows; }            ///< 获取总显示次数
    quint64 totalHides() const { return m_totalHides; }            ///< 获取总隐藏次数
    quint64 totalTitleChanges() const { return m_totalTitleChanges; } ///< 获取总标题变更次数
    quint64 totalTitleClicks() const { return m_totalTitleClicks; }  ///< 获取总标题栏点击次数
    quint64 totalDragStarts() const { return m_totalDragStarts; }    ///< 获取总拖拽开始次数
    quint64 totalSettingsOpens() const { return m_totalSettingsOpens; } ///< 获取总设置面板打开次数
    quint64 totalEmptyStateShows() const { return m_totalEmptyStateShows; } ///< 获取总空状态显示次数
    quint64 totalLoadingShows() const { return m_totalLoadingShows; }  ///< 获取总加载指示器显示次数
    quint64 totalSkeletonShows() const { return m_totalSkeletonShows; } ///< 获取总骨架屏显示次数
    void notifySettingsOpened() { ++m_totalSettingsOpens; }         ///< 递增设置面板打开计数
    void resetPanelStatistics();                                    ///< 重置面板统计计数器
};

#endif // BASE_PANEL_H
