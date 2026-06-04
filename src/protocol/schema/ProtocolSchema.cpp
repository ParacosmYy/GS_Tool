/**
 * @file ProtocolSchema.cpp
 * @brief 自定义协议帧结构定义实现 — 构造/析构/从文件加载
 *
 * 从 JSON 文件加载协议帧结构定义。
 * 字段访问、JSON 序列化、校验类型转换与统计计数器
 * 已拆分至 ProtocolSchemaFields.cpp 和 ProtocolSchemaStats.cpp。
 * JSON字节数据解析(loadFromJsonData)已拆分至 ProtocolSchemaParse.cpp。
 * setter方法(setName/setFraming/addField/setValid)已移至 ProtocolSchemaConfig.cpp。
 */

#include "protocol/schema/ProtocolSchema.h"

#include <QFile>
#include <QJsonDocument>

/** @brief 构造函数 @param parent 父对象指针 */
ProtocolSchema::ProtocolSchema(QObject *parent)
    : QObject(parent)
    , m_valid(false)
{
}

/** @brief 析构函数 */
ProtocolSchema::~ProtocolSchema() = default;

/** @brief 从JSON文件加载协议定义 @param filePath JSON文件的完整路径 @return 加载成功返回true，否则返回false */
bool ProtocolSchema::loadFromJson(const QString &filePath)
{
    ++m_totalLoads;  // 累计加载计数(含成功和失败)
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_valid = false;
        m_lastError = tr("无法打开文件: %1").arg(filePath);
        ++m_totalSchemaErrors;
        return false;
    }

    const QByteArray data = file.readAll();
    file.close();

    if (data.isEmpty()) {
        m_valid = false;
        m_lastError = tr("文件内容为空: %1").arg(filePath);
        ++m_totalSchemaErrors;
        return false;
    }

    return loadFromJsonData(data);
}
