/**
 * @file WidgetFactory.cpp
 * @brief 控件工厂的实现文件，提供基于类型名称的控件注册与创建机制
 */

#include "core/factory/WidgetFactory.h"
#include <QWidget>

/** @brief 构造函数，初始化控件工厂 @param parent 父对象指针 */
WidgetFactory::WidgetFactory(QObject *parent) : QObject(parent) {}

/** @brief 析构函数，使用默认实现 */
WidgetFactory::~WidgetFactory() = default;

/** @brief 注册一个新的控件类型及其创建函数 @param typeName 控件类型名称，作为创建时的查找键 @param creator 控件创建回调函数，接收父控件指针并返回新创建的控件实例 */
void WidgetFactory::registerType(const QString &typeName, WidgetCreator creator)
{
    m_creators[typeName] = creator;
    ++m_totalRegistrations;
}

/** @brief 注销指定名称的控件类型，移除其创建函数 @param typeName 要注销的控件类型名称 */
void WidgetFactory::unregisterType(const QString &typeName)
{
    m_creators.remove(typeName);
}

/** @brief 根据类型名称创建控件实例 @param typeName 已注册的控件类型名称 @param parent 新控件的父控件指针 @return 成功时返回创建的控件指针，类型未注册时返回nullptr */
QWidget* WidgetFactory::create(const QString &typeName, QWidget *parent) const
{
    auto it = m_creators.constFind(typeName);
    if (it != m_creators.constEnd()) {
        ++m_totalCreations;
        ++m_widgetsByType[typeName];
        QWidget* widget = it.value()(parent);
        // 监听控件销毁以追踪销毁计数
        if (widget) {
            connect(widget, &QObject::destroyed, this, [this]() {
                ++m_totalWidgetsDestroyed;
            });
        }
        return widget;
    }
    ++m_totalCreationFailures;
    return nullptr;
}

/** @brief 检查指定名称的控件类型是否已注册 @param typeName 待检查的控件类型名称 @return 已注册返回true，否则返回false */
bool WidgetFactory::isRegistered(const QString &typeName) const
{
    return m_creators.contains(typeName);
}

/** @brief 获取所有已注册的控件类型名称列表 @return 已注册类型名称的字符串列表 */
QStringList WidgetFactory::registeredTypes() const
{
    return m_creators.keys();
}

/** @brief 重置所有统计计数器(注册/创建/失败/销毁/类型分布) */
void WidgetFactory::resetFactoryStatistics()
{
    m_totalRegistrations = 0;
    m_totalCreations = 0;
    m_totalCreationFailures = 0;
    m_totalWidgetsDestroyed = 0;
    m_widgetsByType.clear();
}