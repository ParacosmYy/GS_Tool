/**
 * @file PanelDescriptor.h
 * @brief 主窗口内置面板的运行时元数据描述
 *
 * 仅供 core/panels 内部收敛面板包装、导航映射和布局列表使用。
 * 第一阶段允许持有 QWidget*，因此不作为 interfaces 层契约对外暴露。
 */
#ifndef PANEL_DESCRIPTOR_H
#define PANEL_DESCRIPTOR_H

#include <QWidget>

/**
 * @brief 面板包装和展示策略
 *
 * 该枚举表达 UI 编排语义，不绑定具体业务面板实现。
 */
enum class PanelWrapperPolicy {
    Wrapped,        ///< 使用标准 BasePanel 外壳
    RawPersistent,  ///< 原始控件常驻显示，如 Terminal
    Overlay,        ///< 叠加层控件，如 TerminalSearchBar
    FixedBar,       ///< 固定底栏控件，如 QuickCommandBar
    Floating        ///< 浮动窗口或对话框
};

/**
 * @brief 主窗口内置面板的运行时描述
 *
 * 字段只保存稳定 UI 事实和运行时 raw widget 指针。
 * 图标字段使用 Lucide 逻辑名，不保存资源路径、颜色或 QIcon。
 */
struct PanelDescriptor {
    const char* id = "";          ///< 稳定英文 ID，如 "serial.config"
    const char* objectName = "";  ///< raw widget objectName
    const char* groupKey = "";    ///< 导航分组翻译 key
    const char* titleKey = "";    ///< 面板标题翻译 key
    const char* iconName = "";    ///< Lucide 逻辑名
    QWidget* rawWidget = nullptr;  ///< 已创建的原始面板
    PanelWrapperPolicy wrapperPolicy = PanelWrapperPolicy::Wrapped;
    int navOrder = -1;             ///< 导航顺序，-1 表示不参与导航排序
    int stackOrder = -1;           ///< 面板栈顺序，-1 表示不参与布局排序
    bool navVisible = true;        ///< 是否出现在导航树
    bool includeInPanelStack = true; ///< 是否加入右侧面板栈布局
};

#endif // PANEL_DESCRIPTOR_H
