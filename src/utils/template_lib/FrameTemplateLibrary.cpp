/**
 * @file FrameTemplateLibrary.cpp
 * @brief 帧模板库实现 — 模板增删查改、帧组装、校验和计算、JSON 导入导出
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/template_lib/FrameTemplateLibrary.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "utils/crypto/CRC.h"

// ============================================================
// 构造 / 析构
// ============================================================

/**
 * @brief 构造函数 — 初始化 objectName
 */
FrameTemplateLibrary::FrameTemplateLibrary(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("FrameTemplateLibrary"));
}

/** @brief 析构函数 — QMap 自动释放 */
FrameTemplateLibrary::~FrameTemplateLibrary() = default;

// ============================================================
// 模板 CRUD
// ============================================================

/**
 * @brief 添加模板 — 同名模板拒绝添加
 * @param tmpl 模板定义
 * @return true 添加成功
 */
bool FrameTemplateLibrary::addTemplate(const FrameTemplate &tmpl)
{
    if (tmpl.name.isEmpty() || m_templates.contains(tmpl.name)) {
        return false;
    }
    m_templates.insert(tmpl.name, tmpl);
    refreshStats();
    emit templateAdded(tmpl.name);
    return true;
}

/**
 * @brief 移除模板
 * @param name 模板名称
 * @return true 移除成功
 */
bool FrameTemplateLibrary::removeTemplate(const QString &name)
{
    if (!m_templates.contains(name)) {
        return false;
    }
    m_templates.remove(name);
    refreshStats();
    emit templateRemoved(name);
    return true;
}

/**
 * @brief 按名称查询模板
 * @param name 模板名称
 * @return 模板指针（不存在返回 nullptr）
 */
const FrameTemplateLibrary::FrameTemplate *FrameTemplateLibrary::template_(
    const QString &name) const
{
    auto it = m_templates.constFind(name);
    if (it == m_templates.constEnd()) {
        return nullptr;
    }
    return &it.value();
}

/**
 * @brief 获取所有模板列表
 * @return 模板列表的常引用
 */
const QList<FrameTemplateLibrary::FrameTemplate> &FrameTemplateLibrary::allTemplates() const
{
    return m_templates.values();
}

/**
 * @brief 获取所有分类名称（去重）
 * @return 分类字符串列表
 */
QStringList FrameTemplateLibrary::categories() const
{
    QStringList result;
    for (auto it = m_templates.constBegin(); it != m_templates.constEnd(); ++it) {
        const QString &cat = it.value().category;
        if (!cat.isEmpty() && !result.contains(cat)) {
            result.append(cat);
        }
    }
    return result;
}

/**
 * @brief 按分类筛选模板
 * @param category 分类名称
 * @return 匹配的模板列表
 */
QList<FrameTemplateLibrary::FrameTemplate> FrameTemplateLibrary::findByCategory(
    const QString &category) const
{
    QList<FrameTemplate> result;
    for (auto it = m_templates.constBegin(); it != m_templates.constEnd(); ++it) {
        if (it.value().category == category) {
            result.append(it.value());
        }
    }
    return result;
}

// ============================================================
// 帧组装
// ============================================================

/**
 * @brief 使用模板默认值构建帧
 *
 * 组装流程: header + fields[0..n] + checksum + footer。
 * checksumOffset=-1 时校验值自动追加在字段之后、footer 之前。
 *
 * @param templateName 模板名称
 * @return 完整帧数据
 */
QByteArray FrameTemplateLibrary::buildFrame(const QString &templateName)
{
    const FrameTemplate *tmpl = template_(templateName);
    if (!tmpl) {
        return {};
    }

    /* 拼接: header + 所有字段默认值 */
    QByteArray frame;
    frame.append(tmpl->header);
    for (const auto &field : tmpl->fields) {
        frame.append(field.value);
    }

    /* 计算校验和 */
    applyChecksum(frame, tmpl->checksumType, tmpl->checksumOffset);

    /* 追加帧尾 */
    frame.append(tmpl->footer);

    ++m_stats.totalFramesBuilt;
    emit frameBuilt(templateName, frame);
    return frame;
}

/**
 * @brief 使用自定义字段值构建帧
 * @param templateName 模板名称
 * @param fieldValues 自定义字段值列表（按 name 匹配模板字段）
 * @return 完整帧数据
 */
QByteArray FrameTemplateLibrary::buildFrameWithValues(
    const QString &templateName,
    const QList<FieldValue> &fieldValues)
{
    const FrameTemplate *tmpl = template_(templateName);
    if (!tmpl) {
        return {};
    }

    /* 将自定义值构建为 name→value 的快速查找表 */
    QMap<QString, QByteArray> valueMap;
    for (const auto &fv : fieldValues) {
        valueMap.insert(fv.name, fv.value);
    }

    /* 拼接: header + 字段（优先使用自定义值，回退到默认值） */
    QByteArray frame;
    frame.append(tmpl->header);
    for (const auto &field : tmpl->fields) {
        auto it = valueMap.constFind(field.name);
        if (it != valueMap.constEnd()) {
            frame.append(it.value());
        } else {
            frame.append(field.value);
        }
    }

    /* 计算校验和 */
    applyChecksum(frame, tmpl->checksumType, tmpl->checksumOffset);

    /* 追加帧尾 */
    frame.append(tmpl->footer);

    ++m_stats.totalFramesBuilt;
    emit frameBuilt(templateName, frame);
    return frame;
}

// ============================================================
// 校验和
// ============================================================

/**
 * @brief 计算校验和并写入帧数据
 *
 * 支持算法:
 * - "XOR8":         所有字节异或，1字节
 * - "Sum8":         所有字节累加取低8位，1字节
 * - "CRC16-Modbus": CRC-16/Modbus，2字节（小端序）
 * - "CRC32":        CRC-32，4字节（小端序）
 * - "" 或其它:      不计算
 *
 * @param frame 帧数据（会被原地修改）
 * @param checksumType 校验算法名称
 * @param checksumOffset 写入偏移（-1=自动在当前位置追加）
 */
void FrameTemplateLibrary::applyChecksum(QByteArray &frame,
                                         const QString &checksumType,
                                         int checksumOffset) const
{
    if (checksumType.isEmpty()) {
        return;
    }

    int csSize = checksumSize(checksumType);
    if (csSize <= 0) {
        return;
    }

    /* -1 表示尾部追加: 先扩展空间再计算 */
    if (checksumOffset < 0) {
        checksumOffset = frame.size();
        frame.resize(frame.size() + csSize);
    } else {
        /* 确保帧足够长以容纳校验值 */
        if (checksumOffset + csSize > frame.size()) {
            frame.resize(checksumOffset + csSize);
        }
    }

    /* 对 offset 之前的数据计算校验和 */
    QByteArray payload = frame.left(checksumOffset);

    if (checksumType == QLatin1String("XOR8")) {
        quint8 val = 0x00;
        for (int i = 0; i < payload.size(); ++i) {
            val ^= static_cast<quint8>(payload.at(i));
        }
        frame[checksumOffset] = static_cast<char>(val);

    } else if (checksumType == QLatin1String("Sum8")) {
        quint8 val = CRC::checksum(payload);
        frame[checksumOffset] = static_cast<char>(val);

    } else if (checksumType == QLatin1String("CRC16-Modbus")) {
        quint16 crc = CRC::crc16Modbus(payload);
        frame[checksumOffset]     = static_cast<char>(crc & 0xFF);
        frame[checksumOffset + 1] = static_cast<char>((crc >> 8) & 0xFF);

    } else if (checksumType == QLatin1String("CRC32")) {
        quint32 crc = CRC::crc32(payload);
        frame[checksumOffset]     = static_cast<char>(crc & 0xFF);
        frame[checksumOffset + 1] = static_cast<char>((crc >> 8) & 0xFF);
        frame[checksumOffset + 2] = static_cast<char>((crc >> 16) & 0xFF);
        frame[checksumOffset + 3] = static_cast<char>((crc >> 24) & 0xFF);
    }
}

/**
 * @brief 返回指定校验算法的校验值字节数
 * @param checksumType 校验算法名称
 * @return 字节数（未知算法返回 0）
 */
int FrameTemplateLibrary::checksumSize(const QString &checksumType) const
{
    if (checksumType == QLatin1String("XOR8")
        || checksumType == QLatin1String("Sum8")) {
        return 1;
    }
    if (checksumType == QLatin1String("CRC16-Modbus")) {
        return 2;
    }
    if (checksumType == QLatin1String("CRC32")) {
        return 4;
    }
    return 0;
}

// ============================================================
// JSON 导入 / 导出
// ============================================================

/**
 * @brief 将整个模板库导出为 JSON 文件
 *
 * JSON 结构:
 * @code
 * {
 *   "version": 1,
 *   "templates": [ { "name": "...", "category": "...", ... }, ... ]
 * }
 * @endcode
 *
 * @param filePath 目标文件路径
 * @return true 导出成功
 */
bool FrameTemplateLibrary::exportLibrary(const QString &filePath) const
{
    QJsonObject root;
    root[QStringLiteral("version")] = 1;

    QJsonArray arr;
    for (auto it = m_templates.constBegin(); it != m_templates.constEnd(); ++it) {
        const auto &tmpl = it.value();
        QJsonObject obj;
        obj[QStringLiteral("name")]          = tmpl.name;
        obj[QStringLiteral("category")]      = tmpl.category;
        obj[QStringLiteral("header")]        = QString::fromLatin1(tmpl.header.toHex());
        obj[QStringLiteral("footer")]        = QString::fromLatin1(tmpl.footer.toHex());
        obj[QStringLiteral("checksumType")]  = tmpl.checksumType;
        obj[QStringLiteral("checksumOffset")] = tmpl.checksumOffset;
        obj[QStringLiteral("description")]   = tmpl.description;

        QJsonArray fieldsArr;
        for (const auto &fv : tmpl.fields) {
            QJsonObject fvObj;
            fvObj[QStringLiteral("name")]        = fv.name;
            fvObj[QStringLiteral("value")]       = QString::fromLatin1(fv.value.toHex());
            fvObj[QStringLiteral("description")] = fv.description;
            fieldsArr.append(fvObj);
        }
        obj[QStringLiteral("fields")] = fieldsArr;
        arr.append(obj);
    }
    root[QStringLiteral("templates")] = arr;

    QJsonDocument doc(root);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    ++m_stats.totalExports;
    return true;
}

/**
 * @brief 从 JSON 文件导入模板（追加，不覆盖同名模板）
 * @param filePath JSON 文件路径
 * @return true 文件读取成功
 */
bool FrameTemplateLibrary::importLibrary(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    file.close();
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        return false;
    }

    QJsonObject root = doc.object();
    QJsonArray arr = root[QStringLiteral("templates")].toArray();
    for (const QJsonValue &val : arr) {
        QJsonObject obj = val.toObject();
        FrameTemplate tmpl;
        tmpl.name           = obj[QStringLiteral("name")].toString();
        tmpl.category       = obj[QStringLiteral("category")].toString();
        tmpl.header         = QByteArray::fromHex(obj[QStringLiteral("header")].toString().toLatin1());
        tmpl.footer         = QByteArray::fromHex(obj[QStringLiteral("footer")].toString().toLatin1());
        tmpl.checksumType   = obj[QStringLiteral("checksumType")].toString();
        tmpl.checksumOffset = obj[QStringLiteral("checksumOffset")].toInt(-1);
        tmpl.description    = obj[QStringLiteral("description")].toString();

        QJsonArray fieldsArr = obj[QStringLiteral("fields")].toArray();
        for (const QJsonValue &fvVal : fieldsArr) {
            QJsonObject fvObj = fvVal.toObject();
            FieldValue fv;
            fv.name        = fvObj[QStringLiteral("name")].toString();
            fv.value       = QByteArray::fromHex(fvObj[QStringLiteral("value")].toString().toLatin1());
            fv.description = fvObj[QStringLiteral("description")].toString();
            tmpl.fields.append(fv);
        }

        /* 追加，同名跳过 */
        addTemplate(tmpl);
    }

    ++m_stats.totalImports;
    refreshStats();
    return true;
}

// ============================================================
// 统计刷新
// ============================================================

/** @brief 刷新内部派生统计字段 */
void FrameTemplateLibrary::refreshStats()
{
    m_stats.totalTemplates = static_cast<quint64>(m_templates.size());

    /* 分类去重计数 */
    QSet<QString> cats;
    quint64 maxFields = 0;
    for (auto it = m_templates.constBegin(); it != m_templates.constEnd(); ++it) {
        const auto &tmpl = it.value();
        if (!tmpl.category.isEmpty()) {
            cats.insert(tmpl.category);
        }
        quint64 fc = static_cast<quint64>(tmpl.fields.size());
        if (fc > maxFields) {
            maxFields = fc;
        }
    }
    m_stats.totalCategories = static_cast<quint64>(cats.size());
    m_stats.maxFieldsInTemplate = maxFields;
}
