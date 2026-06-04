/**
 * @file BasePanel.h
 * @brief BasePanel容器组件 — 为所有功能面板提供统一的标题栏、折叠和动画
 *
 * BasePanel包裹一个内容QWidget，在顶部添加统一的标题栏:
 *   [icon(16px)] [标题文字(bold 13px)] [弹性空间] [折叠按钮(chevron)]
 * 标题栏下方是内容区域，支持折叠/展开。
 * 面板显示/隐藏时带有淡入+滑入/淡出+滑出动画。
 *
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

/**
 * @brief BasePanel容器 — 统一面板视觉外壳
 *
 * 职责:
 *   1. 显示统一的标题栏(图标+标题+折叠按钮)
 *   2. 内容区域的折叠/展开
 *   3. 面板显示/隐藏的滑入滑出动画
 *   4. 微阴影效果(通过paintEvent)
 *
 * 不包含任何业务逻辑，纯表现层组件。
 */
class BasePanel : public QWidget {
    Q_OBJECT
    Q_PROPERTY(qreal panelOpacity READ panelOpacity WRITE setPanelOpacity)

public:
    /**
     * @brief 构造BasePanel容器
     * @param content 被包裹的内容面板(QWidget子类)，BasePanel会成为其新的parent
     * @param title 标题栏显示的标题文字(必须使用tr()包裹)
     * @param parent 父控件
     */
    explicit BasePanel(QWidget* content, const QString& title,
                       QWidget* parent = nullptr);

    /** @brief 获取被包裹的原始内容面板 */
    QWidget* contentWidget() const;

    /** @brief 设置/获取标题栏标题 */
    void setTitle(const QString& title);
    QString title() const;

    /**
     * @brief 设置标题栏图标名称(Lucide图标名)
     * @param name Lucide图标名称，如"cable"、"bluetooth"
     * @note 暂用QLabel显示首字符作为占位，后续接入IconManager
     */
    void setIconName(const QString& name);

    /** @brief 设置/获取是否可折叠 */
    void setCollapsible(bool enabled);
    bool isCollapsible() const;

    /** @brief 折叠/展开内容区域 */
    void setCollapsed(bool collapsed);
    bool isCollapsed() const;

    /** @brief 动画透明度属性(供QPropertyAnimation使用) */
    qreal panelOpacity() const;
    void setPanelOpacity(qreal opacity);

    /** @brief 显示空状态占位(隐藏内容面板) */
    void showEmptyState(const QString& title, const QString& description = QString());
    /** @brief 隐藏空状态占位(恢复内容面板) */
    void hideEmptyState();

    /** @brief 显示加载旋转指示器(隐藏内容和空状态) */
    void showLoading();
    /** @brief 隐藏加载指示器(恢复内容面板) */
    void hideLoading();

    /** @brief 显示骨架屏占位(隐藏内容、空状态和加载指示器) */
    void showSkeleton();
    /** @brief 隐藏骨架屏占位(恢复内容面板) */
    void hideSkeleton();

public slots:
    /** @brief 带动画显示面板: opacity 0→1, slide Y:-20→0, 250ms OutCubic */
    void animateShow();
    /** @brief 带动画隐藏面板: opacity 1→0, slide Y:0→-20, 200ms InCubic */
    void animateHide();

signals:
    /** @brief 折叠状态变更信号 */
    void collapsedChanged(bool collapsed);

protected:
    /** @brief 绘制微阴影效果(使用ThemeManager Shadow语义色) */
    void paintEvent(QPaintEvent* event) override;

private:
    void setupInternalLayout();   ///< 构建内部布局(标题栏+内容区域)
    void updateCollapseIcon();    ///< 更新折叠按钮图标方向
    void toggleCollapsed();       ///< 切换折叠状态

protected:
    /** @brief 标题栏事件过滤器，跟踪点击和拖拽 @param watched 目标对象 @param event 事件 @return 是否拦截事件 */
    bool eventFilter(QObject* watched, QEvent* event) override;

    // --- 内部控件 ---
    QWidget*     m_content;       ///< 被包裹的内容面板(不拥有，由PanelManager管理)
    QLabel*      m_iconLabel;     ///< objectName="panelIcon"
    QLabel*      m_titleLabel;    ///< objectName="panelTitle"
    QPushButton* m_collapseBtn;   ///< objectName="collapseButton"
    QWidget*     m_headerBar;     ///< objectName="panelHeaderBar"
    QWidget*     m_contentArea;   ///< objectName="panelContentArea"

    // --- 辅助状态组件 ---
    EmptyStateWidget* m_emptyState = nullptr;     ///< objectName="panelEmptyState"
    LoadingSpinner*   m_loadingSpinner = nullptr;  ///< objectName="panelLoadingSpinner"
    QWidget*          m_skeleton = nullptr;          ///< objectName="panelSkeleton" 骨架屏容器

    // --- 状态 ---
    bool m_collapsible = true;    ///< 是否允许折叠
    bool m_collapsed   = false;   ///< 当前是否折叠
    QString m_iconName;           ///< 图标名称

    // --- 动画 ---
    QPointer<QGraphicsOpacityEffect> m_opacityEffect; ///< 透明度特效
    qreal m_panelOpacity = 1.0;   ///< 动画用透明度属性

    // ---- 统计计数器 ----
    quint64 m_totalToggles = 0;       ///< 总折叠切换次数
    quint64 m_totalExpansions = 0;    ///< 总展开次数
    quint64 m_totalCollapses = 0;     ///< 总折叠次数
    quint64 m_totalShows = 0;         ///< 总显示次数(animateShow触发)
    quint64 m_totalHides = 0;         ///< 总隐藏次数(animateHide触发)
    quint64 m_totalTitleChanges = 0;  ///< 总标题变更次数(setTitle触发)
    quint64 m_totalTitleClicks = 0;   ///< 总标题栏点击次数
    quint64 m_totalDragStarts = 0;    ///< 总拖拽开始次数(标题栏拖拽)
    quint64 m_totalSettingsOpens = 0; ///< 总设置面板打开次数

public:
    /** @brief 获取总折叠切换次数 @return 切换计数 */
    quint64 totalToggles() const { return m_totalToggles; }
    /** @brief 获取总展开次数 @return 展开计数 */
    quint64 totalExpansions() const { return m_totalExpansions; }
    /** @brief 获取总折叠次数 @return 折叠计数 */
    quint64 totalCollapses() const { return m_totalCollapses; }
    /** @brief 获取总显示次数(animateShow触发) @return 显示计数 */
    quint64 totalShows() const { return m_totalShows; }
    /** @brief 获取总隐藏次数(animateHide触发) @return 隐藏计数 */
    quint64 totalHides() const { return m_totalHides; }
    /** @brief 获取总标题变更次数 @return 标题变更计数 */
    quint64 totalTitleChanges() const { return m_totalTitleChanges; }
    /** @brief 获取总标题栏点击次数 @return 点击计数 */
    quint64 totalTitleClicks() const { return m_totalTitleClicks; }
    /** @brief 获取总拖拽开始次数 @return 拖拽开始计数 */
    quint64 totalDragStarts() const { return m_totalDragStarts; }
    /** @brief 获取总设置面板打开次数 @return 设置打开计数 */
    quint64 totalSettingsOpens() const { return m_totalSettingsOpens; }
    /** @brief 递增设置面板打开计数(外部调用者触发设置时使用) */
    void notifySettingsOpened() { ++m_totalSettingsOpens; }
    /** @brief 重置面板统计计数器 */
    void resetPanelStatistics();
};

#endif // BASE_PANEL_H
