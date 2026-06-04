/**
 * @file NavigationController.h
 * @brief 导航控制器 - 管理导航树和面板切换动画
 */

#ifndef NAVIGATION_CONTROLLER_H
#define NAVIGATION_CONTROLLER_H

#include <QObject>
#include <QVector>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QPointer>

class QTreeView;
class QLabel;
class QWidget;
class QSplitter;

/** @brief 导航树->面板映射条目。数据驱动的面板查找结构，消除if-else链。category=分组翻译键, name=面板翻译键, widget=面板指针 */
struct NavPanelMapping {
    const char* category;   ///< 分组翻译键（如 "连接"/"终端"/"图表"/"协议"/"工具"/"调试"/"系统"）
    const char* name;       ///< 面板翻译键，传给 tr() 进行运行时翻译匹配
    QWidget* widget;        ///< 目标面板指针
};

/** @brief 导航控制器 - 导航树构建、面板切换滑动动画、呼吸动画。动画规范: 旧面板200ms InCubic滑出+淡出, 新面板250ms OutCubic滑入+淡入。设计模式: 数据驱动/动画封装 */
class NavigationController : public QObject {
    Q_OBJECT

public:
    explicit NavigationController(QObject* parent = nullptr); ///< 构造导航控制器
    ~NavigationController() override;       ///< 析构，清理动画资源
    void buildNavTree(QTreeView* navTree, const QVector<NavPanelMapping>& mappings); ///< 构建导航树模型并展开全部
    void switchToPanel(QWidget* newPanel);  ///< 面板切换(带滑入滑出动画，动画期间禁用导航树)
    QVector<QWidget*> allSwitchablePanels() const; ///< 收集所有可切换面板widget
    void startBreathingAnimation(QLabel* statusLabel); ///< 启动呼吸动画(1500ms循环, opacity 0.3<->1.0)
    void stopBreathingAnimation(QLabel* statusLabel); ///< 停止呼吸动画(恢复完全不透明)
    void setCurrentPanel(QWidget* panel);   ///< 设置当前面板(初始化用，不触发动画)
    QWidget* lookupPanel(const QString& translatedName) const; ///< 通过翻译后名称查找panel
    const QVector<NavPanelMapping>& mappings() const { return m_navPanelMappings; } ///< 获取映射表
    int currentPanelIndex() const;          ///< 当前面板在映射表中的索引(0~N-1, -1=未找到)
    bool restorePanelByIndex(int index);    ///< 通过索引恢复面板(启动/会话恢复用，无动画)

    void onBreakpointNavCollapse(bool collapsed, QSplitter* splitter, int savedWidth); ///< 响应断点变化调整导航树
    quint64 totalNavigations() const;        ///< 导航切换总次数(含所有switchToPanel)
    quint64 totalPanelSwitches() const;      ///< 面板实际变更总次数(目标!=当前时)
    quint64 totalTreeExpansions() const;     ///< 导航树展开/折叠操作总次数
    quint64 totalSearches() const;           ///< 导航搜索总次数
    quint64 totalBreathingStarts() const;    ///< 呼吸动画启动总次数
    quint64 totalBreathingStops() const;     ///< 呼吸动画停止总次数
    quint64 totalCategoryClicks() const;     ///< 分类节点点击总次数
    quint64 totalRestoresByIndex() const;    ///< 通过索引恢复面板总次数(会话恢复)
    quint64 totalNavTreeRebuilds() const;    ///< 导航树重建总次数(buildNavTree调用)
    void resetNavigationStatistics();        ///< 重置所有导航统计计数器

private slots:
    void onThemeChanged();                   ///< 主题切换时刷新导航树圆点图标颜色

private:
    void animateSlideOut(QWidget* oldPanel, QParallelAnimationGroup* group); ///< 旧面板滑出+淡出: pos (0,0)->(-width,0) 200ms InCubic
    void animateSlideIn(QWidget* newPanel, QParallelAnimationGroup* group); ///< 新面板滑入+淡入: pos (width,0)->(0,0) 250ms OutCubic
    int parentContainerWidth(QWidget* panel) const; ///< 获取面板父容器宽度作为滑动距离

    QVector<NavPanelMapping> m_navPanelMappings; ///< 导航面板映射表(数据驱动)
    QWidget* m_currentPanel = nullptr;       ///< 当前显示的面板
    bool m_panelSwitching = false;           ///< 是否正在执行面板切换动画
    QTreeView* m_navTree = nullptr;          ///< 导航树视图指针
    QParallelAnimationGroup* m_switchAnimGroup = nullptr; ///< 当前面板切换动画组
    QSequentialAnimationGroup* m_breathingAnim = nullptr; ///< 连接状态呼吸动画
    QPointer<QGraphicsOpacityEffect> m_connStatusEffect; ///< 连接状态标签透明度效果

    // ---- 统计计数器 ----
    quint64 m_totalNavigations = 0;      ///< 导航切换总次数(含所有switchToPanel调用)
    quint64 m_totalPanelSwitches = 0;    ///< 面板实际变更次数(目标面板与当前不同时)
    quint64 m_totalTreeExpansions = 0;   ///< 导航树展开/折叠操作总次数
    quint64 m_totalSearches = 0;         ///< 导航搜索总次数
    quint64 m_totalBreathingStarts = 0; ///< 呼吸动画启动总次数
    quint64 m_totalBreathingStops = 0;  ///< 呼吸动画停止总次数
    quint64 m_totalCategoryClicks = 0;  ///< 分类节点点击总次数
    quint64 m_totalRestoresByIndex = 0; ///< 通过索引恢复面板总次数(会话恢复)
    quint64 m_totalNavTreeRebuilds = 0; ///< 导航树重建总次数(buildNavTree调用)
};

#endif // NAVIGATION_CONTROLLER_H
