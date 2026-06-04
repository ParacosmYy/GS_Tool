/**
 * @file ProtocolTemplateLibraryBuiltins.cpp
 * @brief 协议模板库 — 内置模板初始化实现
 *
 * 从 ProtocolTemplateLibrary.cpp 拆分而来，包含
 * JustFloat/FireWater/ModbusRTU/COBS/SLIP 内置协议模板构造。
 */

#include "protocol/schema/ProtocolTemplateLibrary.h"
#include "protocol/schema/ProtocolSchema.h"

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
