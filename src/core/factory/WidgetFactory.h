/**
 * @file WidgetFactory.h
 * @brief 控件工厂，通过注册创建函数实现按类型名称动态创建QWidget
 */
#pragma once
#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <functional>
#include <memory>

class QWidget;

/** @brief 控件创建函数类型，接受父控件指针并返回新创建的控件 */
using WidgetCreator = std::function<QWidget*(QWidget*)>;

/**
 * @class WidgetFactory
 * @brief 控件工厂，支持按名称注册和创建QWidget派生类实例
 */
class WidgetFactory : public QObject {
    Q_OBJECT
public:
    /** @brief 构造函数 @param parent 父对象指针 */
    explicit WidgetFactory(QObject *parent = nullptr);
    /** @brief 析构函数 */
    ~WidgetFactory() override;

    /** @brief 注册控件创建函数 @param typeName 类型名称 @param creator 创建函数 */
    void registerType(const QString &typeName, WidgetCreator creator);
    /** @brief 注销控件类型 @param typeName 类型名称 */
    void unregisterType(const QString &typeName);
    /** @brief 按类型名称创建控件实例 @param typeName 类型名称 @param parent 父控件 @return 新创建的控件指针，未注册时返回nullptr */
    QWidget* create(const QString &typeName, QWidget *parent = nullptr) const;
    /** @brief 查询类型是否已注册 @param typeName 类型名称 @return 是否已注册 */
    bool isRegistered(const QString &typeName) const;
    /** @brief 获取所有已注册的类型名称 @return 类型名称列表 */
    QStringList registeredTypes() const;

    // ---- 统计计数器 ----
    /** @brief 获取累计注册类型次数 @return 注册次数 */
    quint64 totalRegistrations() const { return m_totalRegistrations; }
    /** @brief 获取累计创建控件次数 @return 创建次数 */
    quint64 totalCreations() const { return m_totalCreations; }
    /** @brief 获取累计创建失败次数(未注册类型) @return 失败次数 */
    quint64 totalCreationFailures() const { return m_totalCreationFailures; }
    /** @brief 获取累计销毁控件次数 @return 销毁次数 */
    quint64 totalWidgetsDestroyed() const { return m_totalWidgetsDestroyed; }
    /** @brief 获取按类型名称分类的创建计数 @return 类型名称到创建次数的映射 */
    QMap<QString, quint64> widgetsByType() const { return m_widgetsByType; }
    /** @brief 重置所有统计计数器 */
    void resetFactoryStatistics();

    /**
     * @brief 模板注册方法，自动生成创建函数
     * @tparam T 控件派生类型
     * @param typeName 类型名称
     */
    template<typename T>
    void registerType(const QString &typeName) {
        registerType(typeName, [](QWidget *p) -> QWidget* { return new T(p); });
    }

private:
    QMap<QString, WidgetCreator> m_creators;  ///< 类型名称到创建函数的映射

    // ---- 统计 ----
    quint64 m_totalRegistrations = 0;      ///< 统计: 累计类型注册次数
    mutable quint64 m_totalCreations = 0;  ///< 统计: 累计控件创建成功次数
    mutable quint64 m_totalCreationFailures = 0; ///< 统计: 累计控件创建失败次数
    mutable quint64 m_totalWidgetsDestroyed = 0;   ///< 统计: 累计控件销毁次数
    mutable QMap<QString, quint64> m_widgetsByType; ///< 统计: 按类型名称分类的创建计数
};
