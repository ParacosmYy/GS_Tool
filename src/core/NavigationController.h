#ifndef NAVIGATION_CONTROLLER_H
#define NAVIGATION_CONTROLLER_H

#include <QObject>
#include <QVector>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class QTreeView;
class QLabel;
class QWidget;

/**
 * @brief 导航树 → 面板映射条目
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
 * @brief 导航控制器 - 负责导航树构建、面板切换动画、呼吸动画
 *
 * 职责:
 *   1. 构建导航树的数据模型（串口/网络/工具三组）
 *   2. 通过映射表实现面板名称到 QWidget 的查找
 *   3. 执行面板切换动画（淡出旧面板 → 淡入新面板）
 *   4. 管理连接状态呼吸动画（连接中时的脉冲闪烁效果）
 *
 * 设计模式:
 *   - 数据驱动: 使用 NavPanelMapping 映射表替代 if-else 链
 *   - 动画封装: 将 QPropertyAnimation 细节封装在此控制器中
 *
 * 协作关系:
 *   - MainWindow: 调用 buildNavTree() 构建导航树，调用 switchToPanel() 切换面板
 *   - ConnectionController: 通过 MainWindow 间接调用呼吸动画
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
     * 清理呼吸动画相关资源
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
     * @brief 面板切换（带动画）
     * 执行: 淡出旧面板(200ms InCubic) → 隐藏旧面板 → 显示新面板 → 淡入新面板(250ms OutCubic)
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
     * 1500ms 循环, InOutSine, opacity 0.3 ↔ 1.0 脉冲闪烁
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

signals:
    /**
     * @brief 面板切换完成信号
     * 动画结束后发出，可用于外部同步状态
     * @param panel 切换到的目标面板
     */
    void panelSwitched(QWidget* panel);

private:
    /**
     * @brief 淡入动画辅助
     * 250ms OutCubic opacity 0.0 → 1.0
     * 动画完成后自动清除 QGraphicsOpacityEffect 以恢复正常绘制性能
     * @param panel 需要淡入的面板 widget
     */
    void fadeInPanel(QWidget* panel);

    /** @brief 导航面板映射表（数据驱动，消除 if-else 链） */
    QVector<NavPanelMapping> m_navPanelMappings;

    /** @brief 当前显示的面板（用于淡出动画） */
    QWidget* m_currentPanel = nullptr;

    /** @brief 是否正在执行面板切换动画（防止动画期间重复触发切换） */
    bool m_panelSwitching = false;

    /** @brief 连接状态呼吸动画实例（loopCount=-1 无限循环，需手动管理生命周期） */
    QPropertyAnimation* m_breathingAnim = nullptr;

    /** @brief 连接状态标签的透明度效果实例 */
    QGraphicsOpacityEffect* m_connStatusEffect = nullptr;
};

#endif // NAVIGATION_CONTROLLER_H
