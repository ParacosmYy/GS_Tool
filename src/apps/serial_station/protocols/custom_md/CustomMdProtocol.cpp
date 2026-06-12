#include "apps/serial_station/protocols/custom_md/CustomMdProtocol.h"

#include <QtCore/QRegularExpression>

#include "apps/serial_station/SerialStationConstants.h"
#include "utils/crypto/HexConverter.h"

namespace serial_station {
namespace {

const QByteArray kDefaultHeader = QByteArray::fromHex("A55A");
const QByteArray kDefaultFooter = QByteArray::fromHex("0D0A");
constexpr int kLengthFieldBytes = 1;
constexpr int kCommandFieldBytes = 1;
constexpr int kChecksumBytes = 1;
constexpr int kMaxPayloadBytes = 240;

quint8 byteAt(const QByteArray& bytes, int index)
{
    return static_cast<quint8>(bytes.at(index));
}

bool looksLikeTwoDigitHex(const QString& text)
{
    static const QRegularExpression pattern(QStringLiteral("^[0-9A-Fa-f]{1,2}$"));
    return pattern.match(text).hasMatch();
}

} // namespace

QString CustomMdProtocol::name() const
{
    return QStringLiteral("custom_md");
}

QByteArray CustomMdProtocol::buildCommand(const QString& command, const QVariantMap& params) const
{
    quint8 commandCode = 0;
    QByteArray payload;
    QByteArray header = kDefaultHeader;
    QByteArray footer = kDefaultFooter;
    ChecksumMode checksumMode = ChecksumMode::Sum8;

    m_lastError.clear();
    if (!readCommandCode(command, &commandCode)) {
        return {};
    }
    if (!readPayload(params, &payload)) {
        return {};
    }
    if (!readBytesParam(params, QStringLiteral("header"), &header)) {
        return {};
    }
    if (!readBytesParam(params, QStringLiteral("footer"), &footer)) {
        return {};
    }
    if (!readChecksumMode(params, &checksumMode)) {
        return {};
    }

    return buildFrame(header, footer, checksumMode, commandCode, payload);
}

QVector<SerialProtocolEvent> CustomMdProtocol::feed(const QByteArray& data)
{
    QVector<SerialProtocolEvent> events;
    if (data.isEmpty()) {
        return events;
    }

    m_buffer.append(data);
    if (m_buffer.size() > serialStationConstants::kMaxProtocolBufferBytes) {
        events.append(makeErrorEvent(QStringLiteral("Custom MD buffer overflow"), m_buffer));
        m_buffer.clear();
        return events;
    }

    int headerIndex = findHeader();
    while (headerIndex >= 0) {
        if (headerIndex > 0) {
            m_buffer.remove(0, headerIndex);
        }

        const int frameLength = expectedFrameLength();
        if (frameLength <= 0 || m_buffer.size() < frameLength) {
            break;
        }

        const QByteArray frame = m_buffer.left(frameLength);
        if (!hasValidFooter(frame)) {
            events.append(makeErrorEvent(QStringLiteral("Custom MD footer mismatch"), frame));
            m_buffer.remove(0, 1);
        } else if (!hasValidChecksum(frame)) {
            events.append(makeErrorEvent(QStringLiteral("Custom MD checksum mismatch"), frame));
            m_buffer.remove(0, 1);
        } else {
            events.append(makeFrameEvent(frame));
            m_buffer.remove(0, frameLength);
        }

        headerIndex = findHeader();
    }

    if (headerIndex < 0 && m_buffer.size() > kDefaultHeader.size()) {
        const QByteArray tail = m_buffer.right(kDefaultHeader.size() - 1);
        m_buffer = tail;
    }

    return events;
}

void CustomMdProtocol::reset()
{
    m_buffer.clear();
    m_lastError.clear();
}

QString CustomMdProtocol::lastError() const
{
    return m_lastError;
}

bool CustomMdProtocol::readCommandCode(const QString& command, quint8* commandCode) const
{
    bool ok = false;
    int value = 0;
    const QString text = command.trimmed();

    if (text.startsWith(QStringLiteral("0x"), Qt::CaseInsensitive)) {
        value = text.mid(2).toInt(&ok, 16);
    } else if (looksLikeTwoDigitHex(text)) {
        value = text.toInt(&ok, 16);
    } else {
        value = text.toInt(&ok, 10);
    }

    if (!ok || value < 0 || value > 0xFF) {
        setError(QStringLiteral("invalid Custom MD command code"));
        return false;
    }

    *commandCode = static_cast<quint8>(value);
    return true;
}

bool CustomMdProtocol::readPayload(const QVariantMap& params, QByteArray* payload) const
{
    payload->clear();
    if (!params.contains(QStringLiteral("payload"))) {
        return true;
    }

    const QVariant rawPayload = params.value(QStringLiteral("payload"));
    if (rawPayload.typeId() == QMetaType::QByteArray) {
        *payload = rawPayload.toByteArray();
    } else if (rawPayload.canConvert<QString>()) {
        const QString text = rawPayload.toString();
        if (!text.trimmed().isEmpty() && !HexConverter::isValidHex(text)) {
            setError(QStringLiteral("invalid Custom MD payload hex"));
            return false;
        }
        *payload = HexConverter::fromHexString(text);
    } else {
        setError(QStringLiteral("invalid Custom MD payload type"));
        return false;
    }

    if (payload->size() > kMaxPayloadBytes) {
        setError(QStringLiteral("Custom MD payload is too large"));
        return false;
    }
    return true;
}

bool CustomMdProtocol::readBytesParam(const QVariantMap& params, const QString& key, QByteArray* bytes) const
{
    if (!params.contains(key)) {
        return true;
    }

    const QVariant rawValue = params.value(key);
    if (rawValue.typeId() == QMetaType::QByteArray) {
        *bytes = rawValue.toByteArray();
    } else if (rawValue.canConvert<QString>()) {
        const QString text = rawValue.toString();
        if (!HexConverter::isValidHex(text)) {
            setError(QStringLiteral("invalid Custom MD bytes parameter: %1").arg(key));
            return false;
        }
        *bytes = HexConverter::fromHexString(text);
    } else {
        setError(QStringLiteral("invalid Custom MD bytes parameter: %1").arg(key));
        return false;
    }

    if (bytes->isEmpty()) {
        setError(QStringLiteral("empty Custom MD bytes parameter: %1").arg(key));
        return false;
    }
    return true;
}

bool CustomMdProtocol::readChecksumMode(const QVariantMap& params, ChecksumMode* mode) const
{
    *mode = ChecksumMode::Sum8;
    if (!params.contains(QStringLiteral("checksum"))) {
        return true;
    }

    const QString checksum = params.value(QStringLiteral("checksum")).toString().trimmed().toLower();
    if (checksum == QStringLiteral("sum8")) {
        *mode = ChecksumMode::Sum8;
        return true;
    }
    if (checksum == QStringLiteral("none")) {
        *mode = ChecksumMode::None;
        return true;
    }

    setError(QStringLiteral("unsupported Custom MD checksum mode"));
    return false;
}

QByteArray CustomMdProtocol::buildFrame(
    const QByteArray& header,
    const QByteArray& footer,
    ChecksumMode mode,
    quint8 commandCode,
    const QByteArray& payload) const
{
    QByteArray body;
    body.append(static_cast<char>(kCommandFieldBytes + payload.size()));
    body.append(static_cast<char>(commandCode));
    body.append(payload);

    QByteArray frame = header;
    frame.append(body);
    if (mode == ChecksumMode::Sum8) {
        frame.append(static_cast<char>(checksumFor(body)));
    }
    frame.append(footer);
    return frame;
}

quint8 CustomMdProtocol::checksumFor(const QByteArray& body) const
{
    quint16 sum = 0;
    for (char byte : body) {
        sum = static_cast<quint16>(sum + static_cast<quint8>(byte));
    }
    return static_cast<quint8>(sum & 0xFF);
}

int CustomMdProtocol::findHeader() const
{
    return m_buffer.indexOf(kDefaultHeader);
}

int CustomMdProtocol::minimumFrameLength() const
{
    return kDefaultHeader.size()
        + kLengthFieldBytes
        + kCommandFieldBytes
        + kChecksumBytes
        + kDefaultFooter.size();
}

int CustomMdProtocol::expectedFrameLength() const
{
    if (m_buffer.size() < minimumFrameLength()) {
        return 0;
    }

    const int lengthIndex = kDefaultHeader.size();
    const int bodyLength = byteAt(m_buffer, lengthIndex);
    if (bodyLength < kCommandFieldBytes || bodyLength > kCommandFieldBytes + kMaxPayloadBytes) {
        return minimumFrameLength();
    }

    return kDefaultHeader.size()
        + kLengthFieldBytes
        + bodyLength
        + kChecksumBytes
        + kDefaultFooter.size();
}

bool CustomMdProtocol::hasValidFooter(const QByteArray& frame) const
{
    return frame.endsWith(kDefaultFooter);
}

bool CustomMdProtocol::hasValidChecksum(const QByteArray& frame) const
{
    const int bodyStart = kDefaultHeader.size();
    const int bodyLength = byteAt(frame, bodyStart);
    const QByteArray body = frame.mid(bodyStart, kLengthFieldBytes + bodyLength);
    const int checksumIndex = bodyStart + kLengthFieldBytes + bodyLength;
    return checksumFor(body) == byteAt(frame, checksumIndex);
}

SerialProtocolEvent CustomMdProtocol::makeFrameEvent(const QByteArray& frame) const
{
    const int lengthIndex = kDefaultHeader.size();
    const int bodyLength = byteAt(frame, lengthIndex);
    const int commandIndex = lengthIndex + kLengthFieldBytes;
    const int payloadLength = bodyLength - kCommandFieldBytes;
    const int payloadIndex = commandIndex + kCommandFieldBytes;
    const int checksumIndex = payloadIndex + payloadLength;

    SerialProtocolEvent event;
    event.type = serialStationConstants::kCustomMdFrameType;
    event.protocolName = name();
    event.raw = frame;
    event.payload.insert(QStringLiteral("length"), bodyLength);
    event.payload.insert(QStringLiteral("commandCode"), byteAt(frame, commandIndex));
    event.payload.insert(QStringLiteral("payload"), frame.mid(payloadIndex, payloadLength));
    event.payload.insert(QStringLiteral("checksum"), byteAt(frame, checksumIndex));
    return event;
}

SerialProtocolEvent CustomMdProtocol::makeErrorEvent(const QString& message, const QByteArray& raw) const
{
    SerialProtocolEvent event;
    event.type = serialStationConstants::kCustomMdErrorType;
    event.protocolName = name();
    event.raw = raw;
    event.payload.insert(QStringLiteral("message"), message);
    setError(message);
    return event;
}

void CustomMdProtocol::setError(const QString& message) const
{
    m_lastError = message;
}

} // namespace serial_station
