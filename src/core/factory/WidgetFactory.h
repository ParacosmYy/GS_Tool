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
};
