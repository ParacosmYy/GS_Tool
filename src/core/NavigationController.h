#ifndef NAVIGATION_CONTROLLER_H
#define NAVIGATION_CONTROLLER_H

#include <QObject>
#include <QVector>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>

class QTreeView;
class QLabel;
class QWidget;

/**
 * @brief 导航树 -> 面板映射条目
 *
 * 数据驱动的面板查找结构，消除 if-else 链。
 * name: 导航树叶子节点的显示文本（使用裸字符串，运行时通过 tr() 匹配翻译后的值）
 * widget: 对应的面板 QWidget 指针（在 buildNavTree 中绑定）
 */
struct NavPanelMapping {
    const char* name;       ///< 翻译键，传给 tr() 进行运行时翻译匹配
    QWidget* widget;        ///< 目标面板指针
};

/**
 * @brief 导航控制器 - 导航树构建、面板切换滑动动画、呼吸动画
 *
 * 职责: 导航树模型构建 / 面板名称查找 / 滑入滑出切换动画 / 连接状态呼吸动画
 * 动画规范 (CLAUDE.md 6.5): 旧面板 200ms InCubic 滑出+淡出, 新面板 250ms OutCubic 滑入+淡入
 * 设计模式: 数据驱动(NavPanelMapping映射表) / 动画封装(QPropertyAnimation)
 * 协作: MainWindow(调用buildNavTree/switchToPanel) / ConnectionController(间接调用呼吸动画)
 */
class NavigationController : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造导航控制器
     * @param parent 父对象
     */
    explicit NavigationController(QObject* parent = nullptr);

    /**
     * @brief 析构导航控制器
     * 清理呼吸动画和进行中的切换动画相关资源
     */
    ~NavigationController() override;

    /**
     * @brief 构建导航树模型并展开全部节点
     * 创建三个分组: 串口(蓝点)、网络(绿点/黄点)、工具
     * @param navTree 导航树视图控件
     * @param mappings 面板名称到 QWidget 的映射表
     */
    void buildNavTree(QTreeView* navTree, const QVector<NavPanelMapping>& mappings);

    /**
     * @brief 面板切换（带滑入滑出动画）
     *
     * 执行滑动 + 淡入淡出动画:
     *   旧面板: pos (0,0)->(-width,0) 200ms InCubic + opacity 1->0
     *   新面板: pos (width,0)->(0,0) 250ms OutCubic + opacity 0->1
     * 动画期间禁用导航树防止重复触发。
     *
     * @param newPanel 目标面板 widget
     */
    void switchToPanel(QWidget* newPanel);

    /**
     * @brief 收集所有可切换面板 widget
     * 用于全部隐藏（面板切换前需要先隐藏所有面板）
     * @return 面板 widget 列表
     */
    QVector<QWidget*> allSwitchablePanels() const;

    /**
     * @brief 启动连接状态呼吸动画
     * 1500ms 循环, InOutSine, opacity 0.3 <-> 1.0 脉冲闪烁
     * @param statusLabel 状态标签控件
     */
    void startBreathingAnimation(QLabel* statusLabel);

    /**
     * @brief 停止连接状态呼吸动画
     * 恢复标签为完全不透明状态
     * @param statusLabel 状态标签控件（析构时可传 nullptr）
     */
    void stopBreathingAnimation(QLabel* statusLabel);

    /**
     * @brief 设置当前面板（初始化用，不触发动画）
     * @param panel 当前应显示的面板 widget
     */
    void setCurrentPanel(QWidget* panel);

    /**
     * @brief 通过翻译后的名称查找对应的 panel widget
     * @param translatedName 导航树节点的翻译后文本
     * @return 匹配的面板 widget，未找到返回 nullptr
     */
    QWidget* lookupPanel(const QString& translatedName) const;

    /**
     * @brief 获取映射表（供外部遍历使用）
     * @return 映射表的常引用
     */
    const QVector<NavPanelMapping>& mappings() const { return m_navPanelMappings; }

    /**
     * @brief 获取当前面板在映射表中的索引
     *
     * 遍历映射表找到 m_currentPanel 对应的索引，用于持久化保存上次活跃面板。
     *
     * @return 面板索引（0~N-1），未找到或无当前面板时返回 -1
     */
    int currentPanelIndex() const;

    /** @brief 通过索引恢复面板（启动/会话恢复用，无动画） @return true成功 false越界/空 */
    bool restorePanelByIndex(int index);

private:
    /** @brief 旧面板滑出+淡出: pos (0,0)->(-width,0) 200ms InCubic, opacity 1->0 */
    void animateSlideOut(QWidget* oldPanel, QParallelAnimationGroup* group);

    /** @brief 新面板滑入+淡入: pos (width,0)->(0,0) 250ms OutCubic, opacity 0->1 */
    void animateSlideIn(QWidget* newPanel, QParallelAnimationGroup* group);

    /** @brief 获取面板父容器宽度作为滑动距离 */
    int parentContainerWidth(QWidget* panel) const;

    /** @brief 导航面板映射表（数据驱动，消除 if-else 链） */
    QVector<NavPanelMapping> m_navPanelMappings;

    /** @brief 当前显示的面板（用于淡出动画） */
    QWidget* m_currentPanel = nullptr;

    /** @brief 是否正在执行面板切换动画（防止动画期间重复触发切换） */
    bool m_panelSwitching = false;

    /** @brief 导航树视图指针（动画期间禁用交互，动画完成后恢复） */
    QTreeView* m_navTree = nullptr;

    /** @brief 当前面板切换动画组（用于析构时清理进行中的动画） */
    QParallelAnimationGroup* m_switchAnimGroup = nullptr;

    /** @brief 连接状态呼吸动画实例（loopCount=-1 无限循环，需手动管理生命周期） */
    QPropertyAnimation* m_breathingAnim = nullptr;

    /** @brief 连接状态标签的透明度效果实例 */
    QGraphicsOpacityEffect* m_connStatusEffect = nullptr;
};

#endif // NAVIGATION_CONTROLLER_H
