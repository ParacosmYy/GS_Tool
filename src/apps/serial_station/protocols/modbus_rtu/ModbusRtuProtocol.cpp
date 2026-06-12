#include "apps/serial_station/protocols/modbus_rtu/ModbusRtuProtocol.h"

#include "apps/serial_station/SerialStationConstants.h"
#include "utils/crypto/CRC.h"

namespace serial_station {
namespace {

constexpr quint8 kFunctionReadCoils = 0x01;
constexpr quint8 kFunctionReadDiscreteInputs = 0x02;
constexpr quint8 kFunctionReadHoldingRegisters = 0x03;
constexpr quint8 kFunctionReadInputRegisters = 0x04;
constexpr quint8 kFunctionWriteSingleCoil = 0x05;
constexpr quint8 kFunctionWriteSingleRegister = 0x06;
constexpr quint8 kExceptionMask = 0x80;
constexpr int kMinSlaveId = 1;
constexpr int kMaxSlaveId = 247;
constexpr int kMaxCoilQuantity = 2000;
constexpr int kMaxRegisterQuantity = 125;
constexpr int kFixedFrameLength = 8;
constexpr int kExceptionFrameLength = 5;

quint8 byteAt(const QByteArray& bytes, int index)
{
    return static_cast<quint8>(bytes.at(index));
}

quint16 wordFromBytes(quint8 high, quint8 low)
{
    return static_cast<quint16>((static_cast<quint16>(high) << 8) | low);
}

void appendWord(QByteArray* bytes, quint16 value)
{
    bytes->append(static_cast<char>((value >> 8) & 0xFF));
    bytes->append(static_cast<char>(value & 0xFF));
}

} // namespace

QString ModbusRtuProtocol::name() const
{
    return QStringLiteral("modbus_rtu");
}

QByteArray ModbusRtuProtocol::buildCommand(const QString& command, const QVariantMap& params) const
{
    const QString normalized = command.trimmed().toLower();
    m_lastError.clear();

    if (normalized == QStringLiteral("read_coils")) {
        return buildReadCommand(kFunctionReadCoils, params, kMaxCoilQuantity);
    }
    if (normalized == QStringLiteral("read_discrete_inputs")) {
        return buildReadCommand(kFunctionReadDiscreteInputs, params, kMaxCoilQuantity);
    }
    if (normalized == QStringLiteral("read_holding_registers")) {
        return buildReadCommand(kFunctionReadHoldingRegisters, params, kMaxRegisterQuantity);
    }
    if (normalized == QStringLiteral("read_input_registers")) {
        return buildReadCommand(kFunctionReadInputRegisters, params, kMaxRegisterQuantity);
    }
    if (normalized == QStringLiteral("write_single_coil")) {
        return buildWriteCommand(kFunctionWriteSingleCoil, params, ValueMode::CoilValue);
    }
    if (normalized == QStringLiteral("write_single_register")) {
        return buildWriteCommand(kFunctionWriteSingleRegister, params, ValueMode::RegisterValue);
    }

    setError(QStringLiteral("unsupported Modbus RTU command"));
    return {};
}

QVector<SerialProtocolEvent> ModbusRtuProtocol::feed(const QByteArray& data)
{
    QVector<SerialProtocolEvent> events;
    if (data.isEmpty()) {
        return events;
    }

    m_buffer.append(data);
    if (m_buffer.size() > serialStationConstants::kMaxProtocolBufferBytes) {
        events.append(makeErrorEvent(QStringLiteral("Modbus RTU buffer overflow"), m_buffer));
        m_buffer.clear();
        return events;
    }

    int frameLength = expectedFrameLength();
    while (frameLength > 0 && m_buffer.size() >= frameLength) {
        const QByteArray frame = m_buffer.left(frameLength);
        m_buffer.remove(0, frameLength);

        if (hasValidCrc(frame)) {
            events.append(makeFrameEvent(frame));
        } else {
            events.append(makeErrorEvent(QStringLiteral("Modbus RTU CRC mismatch"), frame));
        }

        frameLength = expectedFrameLength();
    }

    return events;
}

void ModbusRtuProtocol::reset()
{
    m_buffer.clear();
    m_lastError.clear();
}

QString ModbusRtuProtocol::lastError() const
{
    return m_lastError;
}

QByteArray ModbusRtuProtocol::buildReadCommand(quint8 functionCode, const QVariantMap& params, int maxQuantity) const
{
    quint8 slaveId = 0;
    quint16 startAddress = 0;
    quint16 quantity = 0;

    if (!readByteParam(params, QStringLiteral("slaveId"), kMinSlaveId, kMaxSlaveId, &slaveId)) {
        return {};
    }
    if (!readWordParam(params, QStringLiteral("startAddress"), 0, 0xFFFF, &startAddress)) {
        return {};
    }
    if (!readWordParam(params, QStringLiteral("quantity"), 1, maxQuantity, &quantity)) {
        return {};
    }

    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(functionCode));
    appendWord(&frame, startAddress);
    appendWord(&frame, quantity);
    return frameWithCrc(frame);
}

QByteArray ModbusRtuProtocol::buildWriteCommand(quint8 functionCode, const QVariantMap& params, ValueMode mode) const
{
    quint8 slaveId = 0;
    quint16 address = 0;
    quint16 value = 0;

    if (!readByteParam(params, QStringLiteral("slaveId"), kMinSlaveId, kMaxSlaveId, &slaveId)) {
        return {};
    }
    if (!readWordParam(params, QStringLiteral("address"), 0, 0xFFFF, &address)) {
        return {};
    }

    if (mode == ValueMode::CoilValue) {
        if (!readCoilValue(params, &value)) {
            return {};
        }
    } else if (!readWordParam(params, QStringLiteral("value"), 0, 0xFFFF, &value)) {
        return {};
    }

    QByteArray frame;
    frame.append(static_cast<char>(slaveId));
    frame.append(static_cast<char>(functionCode));
    appendWord(&frame, address);
    appendWord(&frame, value);
    return frameWithCrc(frame);
}

bool ModbusRtuProtocol::readByteParam(
    const QVariantMap& params, const QString& key, int minValue, int maxValue, quint8* value) const
{
    quint16 word = 0;
    if (!readWordParam(params, key, minValue, maxValue, &word)) {
        return false;
    }
    *value = static_cast<quint8>(word);
    return true;
}

bool ModbusRtuProtocol::readWordParam(
    const QVariantMap& params, const QString& key, int minValue, int maxValue, quint16* value) const
{
    bool ok = false;
    const int number = params.value(key).toInt(&ok);
    if (!params.contains(key) || !ok || number < minValue || number > maxValue) {
        setError(QStringLiteral("invalid Modbus RTU parameter: %1").arg(key));
        return false;
    }

    *value = static_cast<quint16>(number);
    return true;
}

bool ModbusRtuProtocol::readCoilValue(const QVariantMap& params, quint16* value) const
{
    if (!params.contains(QStringLiteral("value"))) {
        setError(QStringLiteral("invalid Modbus RTU parameter: value"));
        return false;
    }

    const QVariant rawValue = params.value(QStringLiteral("value"));
    if (rawValue.typeId() == QMetaType::Bool) {
        *value = rawValue.toBool() ? 0xFF00 : 0x0000;
        return true;
    }

    quint16 word = 0;
    if (!readWordParam(params, QStringLiteral("value"), 0, 0xFFFF, &word)) {
        return false;
    }
    if (word != 0x0000 && word != 0xFF00) {
        setError(QStringLiteral("invalid Modbus RTU coil value"));
        return false;
    }

    *value = word;
    return true;
}

QByteArray ModbusRtuProtocol::frameWithCrc(const QByteArray& frame) const
{
    QByteArray bytes = frame;
    const quint16 crc = CRC::crc16Modbus(bytes);
    bytes.append(static_cast<char>(crc & 0xFF));
    bytes.append(static_cast<char>((crc >> 8) & 0xFF));
    return bytes;
}

int ModbusRtuProtocol::expectedFrameLength() const
{
    if (m_buffer.size() < kExceptionFrameLength) {
        return 0;
    }

    const quint8 functionCode = byteAt(m_buffer, 1);
    if ((functionCode & kExceptionMask) != 0) {
        return kExceptionFrameLength;
    }

    if (functionCode == kFunctionWriteSingleCoil || functionCode == kFunctionWriteSingleRegister) {
        return kFixedFrameLength;
    }

    if (functionCode >= kFunctionReadCoils && functionCode <= kFunctionReadInputRegisters) {
        if (m_buffer.size() >= kFixedFrameLength && hasValidCrc(m_buffer.left(kFixedFrameLength))) {
            return kFixedFrameLength;
        }

        const int byteCount = byteAt(m_buffer, 2);
        if (byteCount <= 0) {
            return 0;
        }
        return 3 + byteCount + 2;
    }

    return kExceptionFrameLength;
}

bool ModbusRtuProtocol::hasValidCrc(const QByteArray& frame) const
{
    if (frame.size() < 4) {
        return false;
    }

    const QByteArray body = frame.left(frame.size() - 2);
    const quint16 expected = wordFromBytes(byteAt(frame, frame.size() - 1), byteAt(frame, frame.size() - 2));
    return CRC::crc16Modbus(body) == expected;
}

SerialProtocolEvent ModbusRtuProtocol::makeFrameEvent(const QByteArray& frame) const
{
    const quint8 functionCode = byteAt(frame, 1);
    SerialProtocolEvent event;
    event.type = serialStationConstants::kModbusFrameType;
    event.protocolName = name();
    event.raw = frame;
    event.payload.insert(QStringLiteral("slaveId"), byteAt(frame, 0));
    event.payload.insert(QStringLiteral("functionCode"), functionCode);

    if ((functionCode & kExceptionMask) != 0) {
        event.payload.insert(QStringLiteral("exception"), true);
        event.payload.insert(QStringLiteral("exceptionCode"), byteAt(frame, 2));
        return event;
    }

    event.payload.insert(QStringLiteral("exception"), false);
    if (functionCode >= kFunctionReadCoils && functionCode <= kFunctionReadInputRegisters && frame.size() >= 5) {
        const int byteCount = byteAt(frame, 2);
        event.payload.insert(QStringLiteral("byteCount"), byteCount);
        event.payload.insert(QStringLiteral("data"), frame.mid(3, byteCount));
    } else if (frame.size() == kFixedFrameLength) {
        event.payload.insert(QStringLiteral("address"), wordFromBytes(byteAt(frame, 2), byteAt(frame, 3)));
        event.payload.insert(QStringLiteral("value"), wordFromBytes(byteAt(frame, 4), byteAt(frame, 5)));
    }

    return event;
}

SerialProtocolEvent ModbusRtuProtocol::makeErrorEvent(const QString& message, const QByteArray& raw) const
{
    SerialProtocolEvent event;
    event.type = serialStationConstants::kModbusErrorType;
    event.protocolName = name();
    event.raw = raw;
    event.payload.insert(QStringLiteral("message"), message);
    setError(message);
    return event;
}

void ModbusRtuProtocol::setError(const QString& message) const
{
    m_lastError = message;
}

} // namespace serial_station
