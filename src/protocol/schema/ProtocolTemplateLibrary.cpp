/**
 * @file ProtocolTemplateLibrary.cpp
 * @brief 协议模板库实现
 *
 * 管理内置协议模板和用户导入模板，支持按名称加载、
 * JSON 文件导入/导出。内置模板通过编程方式构造。
 */

#include "protocol/schema/ProtocolTemplateLibrary.h"
#include "protocol/schema/ProtocolSchema.h"

#include <QFile>
#include <QJsonDocument>

/* ──────────────────────── 构造 / 析构 ──────────────────────── */

/** @brief 构造函数(初始化内置协议模板) @param parent 父对象指针 */
ProtocolTemplateLibrary::ProtocolTemplateLibrary(QObject *parent)
    : QObject(parent)
{
    initBuiltinTemplates();
}

/** @brief 析构函数，Qt对象树自动释放模板 */
ProtocolTemplateLibrary::~ProtocolTemplateLibrary() = default;

/* ──────────────────────── 公开接口 ──────────────────────── */

/** @brief 获取所有已加载模板名称 @return 模板名称列表 */
QStringList ProtocolTemplateLibrary::builtinTemplateNames() const
{
    return m_templates.keys();
}

/** @brief 按名称加载模板(返回已存在指针) @param name 模板名称 @return 找到返回ProtocolSchema指针，否则nullptr */
ProtocolSchema *ProtocolTemplateLibrary::loadTemplate(const QString &name)
{
    ++m_totalLoads;
    return m_templates.value(name, nullptr);
}

/** @brief 从JSON文件导入用户模板 @param filePath JSON模板文件路径 @return 导入成功返回true */
bool ProtocolTemplateLibrary::importTemplate(const QString &filePath)
{
    auto *schema = new ProtocolSchema(this);
    if (!schema->loadFromJson(filePath) || !schema->isValid()) {
        delete schema;
        return false;
    }
    const QString key = schema->name();
    /* 若已存在同名模板，先移除旧实例 */
    if (auto *old = m_templates.value(key)) {
        old->deleteLater();
    }
    m_templates[key] = schema;
    ++m_totalImports;
    return true;
}

/** @brief 将指定模板导出为JSON文件 @param name 模板名称 @param filePath 导出文件路径 @return 导出成功返回true */
bool ProtocolTemplateLibrary::exportTemplate(const QString &name,
                                             const QString &filePath)
{
    auto *schema = m_templates.value(name, nullptr);
    if (!schema) {
        return false;
    }
    const QJsonDocument doc(schema->toJson());
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        return false;
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    ++m_totalExports;
    return true;
}

/** @brief 获取已加载模板数量 @return 内置+用户导入的模板总数 */
int ProtocolTemplateLibrary::templateCount() const
{
    return m_templates.size();
}

/* ──────────────────── 内置模板初始化 ──────────────────── */

/** @brief 初始化内置协议模板(JustFloat/FireWater/ModbusRTU/COBS/SLIP) */
void ProtocolTemplateLibrary::initBuiltinTemplates()
{
    /* ── JustFloat ── */
    {
        auto *s = new ProtocolSchema(this);
        s->setName(QStringLiteral("JustFloat"));
        ProtocolSchema::FramingRule framing;
        framing.type           = QStringLiteral("fixed_header");
        framing.header         = {0x4A, 0x55, 0x53, 0x54}; // "JUST"
        framing.lengthFieldOffset = 0;
        framing.lengthFieldSize  = 0;
        framing.checksumType   = ProtocolSchema::None;
        s->setFraming(framing);
        s->addField({QStringLiteral("ch1"), 4, 4, QStringLiteral("float")});
        s->addField({QStringLiteral("ch2"), 8, 4, QStringLiteral("float")});
        s->addField({QStringLiteral("ch3"), 12, 4, QStringLiteral("float")});
        s->addField({QStringLiteral("ch4"), 16, 4, QStringLiteral("float")});
        s->setValid(true);
        m_templates[QStringLiteral("JustFloat")] = s;
    }

    /* ── FireWater ── */
    {
        auto *s = new ProtocolSchema(this);
        s->setName(QStringLiteral("FireWater"));
        ProtocolSchema::FramingRule framing;
        framing.type           = QStringLiteral("newline_delimited");
        framing.lengthFieldOffset = 0;
        framing.lengthFieldSize  = 0;
        framing.checksumType   = ProtocolSchema::None;
        s->setFraming(framing);
        s->addField({QStringLiteral("text"), 0, 0,
                     QStringLiteral("text")});
        s->setValid(true);
        m_templates[QStringLiteral("FireWater")] = s;
    }

    /* ── Modbus RTU ── */
    {
        auto *s = new ProtocolSchema(this);
        s->setName(QStringLiteral("Modbus RTU"));
        ProtocolSchema::FramingRule framing;
        framing.type           = QStringLiteral("length_field");
        framing.lengthFieldOffset = 2;
        framing.lengthFieldSize  = 1;
        framing.checksumType   = ProtocolSchema::Crc16Modbus;
        s->setFraming(framing);
        s->addField({QStringLiteral("slave_addr"), 0, 1,
                     QStringLiteral("uint8")});
        s->addField({QStringLiteral("func_code"), 1, 1,
                     QStringLiteral("uint8")});
        s->addField({QStringLiteral("data"), 2, 0,
                     QStringLiteral("bytes")});
        s->setValid(true);
        m_templates[QStringLiteral("Modbus RTU")] = s;
    }

    /* ── COBS ── */
    {
        auto *s = new ProtocolSchema(this);
        s->setName(QStringLiteral("COBS"));
        ProtocolSchema::FramingRule framing;
        framing.type           = QStringLiteral("delimiter");
        framing.header         = {0x00};
        framing.lengthFieldOffset = 0;
        framing.lengthFieldSize  = 0;
        framing.checksumType   = ProtocolSchema::None;
        s->setFraming(framing);
        s->addField({QStringLiteral("payload"), 0, 0,
                     QStringLiteral("bytes")});
        s->setValid(true);
        m_templates[QStringLiteral("COBS")] = s;
    }

    /* ── SLIP ── */
    {
        auto *s = new ProtocolSchema(this);
        s->setName(QStringLiteral("SLIP"));
        ProtocolSchema::FramingRule framing;
        framing.type           = QStringLiteral("delimiter");
        framing.header         = {0xC0};
        framing.lengthFieldOffset = 0;
        framing.lengthFieldSize  = 0;
        framing.checksumType   = ProtocolSchema::None;
        s->setFraming(framing);
        s->addField({QStringLiteral("payload"), 0, 0,
                     QStringLiteral("bytes")});
        s->setValid(true);
        m_templates[QStringLiteral("SLIP")] = s;
    }
}
