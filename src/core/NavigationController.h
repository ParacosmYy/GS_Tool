#ifndef NAVIGATION_CONTROLLER_H
#define NAVIGATION_CONTROLLER_H

#include <QObject>
#include <QVector>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

class QTreeView;
class QLabel;
class QWidget;

// 导航树 → 面板映射条目
// name: 导航树叶子节点的显示文本（使用裸字符串，运行时通过 tr() 匹配翻译后的值）
// widget: 对应的面板 QWidget 指针（在 buildNavPanelMappings 中绑定）
struct NavPanelMapping {
    const char* name;       // 翻译键，传给 tr() 进行运行时翻译匹配
    QWidget* widget;        // 目标面板指针
};

// 导航控制器 - 负责导航树构建、面板切换动画、呼吸动画
// 从 MainWindow 中提取的导航相关逻辑，遵循单一职责原则
class NavigationController : public QObject {
    Q_OBJECT

public:
    explicit NavigationController(QObject* parent = nullptr);
    ~NavigationController() override;

    // 构建导航树模型并展开全部节点（接收外部已建好的映射表）
    void buildNavTree(QTreeView* navTree, const QVector<NavPanelMapping>& mappings);

    // 面板切换: 带淡出旧面板 + 淡入新面板动画
    void switchToPanel(QWidget* newPanel);

    // 收集所有可切换面板widget（用于全部隐藏）
    QVector<QWidget*> allSwitchablePanels() const;

    // 启动/停止连接状态呼吸动画（脉冲闪烁效果）
    void startBreathingAnimation(QLabel* statusLabel);
    void stopBreathingAnimation(QLabel* statusLabel);

    // 设置当前面板（初始化用，不触发动画）
    void setCurrentPanel(QWidget* panel);

    // 通过翻译后的名称查找对应的panel widget（返回nullptr表示未找到）
    QWidget* lookupPanel(const QString& translatedName) const;

    // 获取映射表（供外部遍历使用）
    const QVector<NavPanelMapping>& mappings() const { return m_navPanelMappings; }

signals:
    // 面板切换完成时发出（动画结束后，可用于外部同步状态）
    void panelSwitched(QWidget* panel);

private:
    // 淡入动画辅助: 250ms OutCubic opacity 0.0 → 1.0
    void fadeInPanel(QWidget* panel);

    // 导航面板映射表（数据驱动，消除 if-else 链）
    QVector<NavPanelMapping> m_navPanelMappings;

    // 当前显示的面板（用于淡出动画）
    QWidget* m_currentPanel = nullptr;

    // 是否正在执行面板切换动画（防止动画期间重复触发切换）
    bool m_panelSwitching = false;

    // 连接状态呼吸动画（connecting状态时脉冲闪烁）
    QPropertyAnimation* m_breathingAnim = nullptr;
    QGraphicsOpacityEffect* m_connStatusEffect = nullptr;
};

#endif // NAVIGATION_CONTROLLER_H
