/**
 * @file IPanelProvider.h
 * @brief 面板提供者接口 - 面板注册与创建的统一协议
 *
 * 定义了面板的标准化创建和注册接口，使 PanelManager 可以通过统一的方式
 * 管理所有面板的创建、销毁和生命周期。
 *
 * 零出站依赖: 仅依赖 Qt Core 类型，不 include 任何项目头文件
 *
 * 设计模式:
 *   - 工厂模式: 每种面板类型实现此接口，PanelManager 通过接口创建实例
 *   - 注册模式: 启动时注册所有 PanelProvider，运行时按名称查找
 *
 * 协作关系:
 *   - PanelManager: 持有所有 IPanelProvider 注册项，按需创建面板
 *   - IconNavBar: 从 IPanelProvider 获取面板名称和图标
 *   - CommandPalette: 从 IPanelProvider 注册快捷命令
 */
#ifndef INTERFACES_IPANELPROVIDER_H
#define INTERFACES_IPANELPROVIDER_H

#include <QString>
#include <QIcon>
#include <QWidget>

class CommandPalette;

/**
 * @brief 面板提供者接口 - 面板注册与创建的统一协议
 *
 * 每种面板类型提供一个 IPanelProvider 实现，注册到 PanelManager。
 * PanelManager 通过此接口创建面板实例、获取面板元数据。
 */
class IPanelProvider {
public:
    virtual ~IPanelProvider() = default;

    /** @brief 面板唯一标识名称 (如 "serial_config", "terminal", "chart") */
    virtual QString panelName() const = 0;

    /** @brief 面板显示名称 (用于 UI 展示，支持国际化) */
    virtual QString panelDisplayName() const = 0;

    /**
     * @brief 创建面板实例
     * @param parent 父 Widget，用于 Qt 父子树内存管理
     * @return 新创建的面板 Widget 实例
     */
    virtual QWidget* createPanel(QWidget* parent) = 0;

    /** @brief 面板图标 (用于导航栏/标签页图标) */
    virtual QIcon panelIcon() const = 0;

    /** @brief 面板在导航栏中的排序优先级 (越小越靠前) */
    virtual int panelOrder() const = 0;

    /** @brief 面板所属的分类 (如 "connection", "tools", "debug") */
    virtual QString panelCategory() const = 0;

    /**
     * @brief 向命令面板注册快捷命令
     * @param palette 命令面板实例
     */
    virtual void registerCommands(CommandPalette* palette) = 0;
};

#endif // INTERFACES_IPANELPROVIDER_H
