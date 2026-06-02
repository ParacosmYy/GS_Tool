/**
 * @file ProtocolTemplateLibrary.h
 * @brief 协议模板库
 *
 * 管理内置和用户导入的协议帧结构模板，
 * 支持按名称加载、导入 JSON 文件、导出模板。
 */

#ifndef PROTOCOL_TEMPLATE_LIBRARY_H
#define PROTOCOL_TEMPLATE_LIBRARY_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>

class ProtocolSchema;

/**
 * @class ProtocolTemplateLibrary
 * @brief 协议帧模板管理器
 *
 * 提供常用协议的预设模板（如 Modbus RTU、自定义帧等），
 * 并允许用户导入/导出自定义协议模板。
 */
class ProtocolTemplateLibrary : public QObject
{
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit ProtocolTemplateLibrary(QObject *parent = nullptr);

    /** @brief 析构函数 */
    ~ProtocolTemplateLibrary() override;

    /**
     * @brief 获取所有内置模板名称
     * @return 内置模板名称列表
     */
    QStringList builtinTemplateNames() const;

    /**
     * @brief 按名称加载模板
     * @param name 模板名称
     * @return 加载成功返回 ProtocolSchema 指针，否则返回 nullptr
     */
    ProtocolSchema *loadTemplate(const QString &name);

    /**
     * @brief 从文件导入用户模板
     * @param filePath JSON 模板文件路径
     * @return 导入成功返回 true
     */
    bool importTemplate(const QString &filePath);

    /**
     * @brief 将指定模板导出为 JSON 文件
     * @param name 模板名称
     * @param filePath 导出文件路径
     * @return 导出成功返回 true
     */
    bool exportTemplate(const QString &name, const QString &filePath);

    /**
     * @brief 获取已加载模板数量
     * @return 内置 + 用户导入的模板总数
     */
    int templateCount() const;

private:
    /** @brief 初始化内置协议模板（在构造时调用） */
    void initBuiltinTemplates();

    QMap<QString, ProtocolSchema *> m_templates; ///< 模板名称 → 协议定义映射
};

#endif // PROTOCOL_TEMPLATE_LIBRARY_H
