#include "protocol/decoder/FrameDecoder.h"
#include <QTextCodec>
#include <QtEndian>

FrameDecoder::FrameDecoder(QObject *parent) : QObject(parent) {}
FrameDecoder::~FrameDecoder() = default;

void FrameDecoder::setEncoding(Encoding e) { m_encoding = e; }
void FrameDecoder::setByteOrder(ByteOrder o) { m_byteOrder = o; }

QString FrameDecoder::decodeToString(const QByteArray &data) const {
    switch (m_encoding) {
    case Ascii: return QString::fromLatin1(data);
    case Utf8: return QString::fromUtf8(data);
    case Hex: return QString::fromLatin1(data.toHex(' '));
    case Base64: return QString::fromUtf8(QByteArray::fromBase64(data));
    case Gb2312: {
        auto codec = QTextCodec::codecForName("GB2312");
        return codec ? codec->toUnicode(data) : QString::fromUtf8(data);
    }
    case Big5: {
        auto codec = QTextCodec::codecForName("Big5");
        return codec ? codec->toUnicode(data) : QString::fromUtf8(data);
    }
    case ShiftJis: {
        auto codec = QTextCodec::codecForName("Shift-JIS");
        return codec ? codec->toUnicode(data) : QString::fromUtf8(data);
    }
    case Raw: default: return data.toHex(' ').toUpper();
    }
}

QByteArray FrameDecoder::encodeFromString(const QString &str) const {
    switch (m_encoding) {
    case Ascii: return str.toLatin1();
    case Utf8: return str.toUtf8();
    case Hex: return QByteArray::fromHex(str.toLatin1());
    case Base64: return str.toUtf8().toBase64();
    case Gb2312: {
        auto codec = QTextCodec::codecForName("GB2312");
        return codec ? codec->fromUnicode(str) : str.toUtf8();
    }
    default: return str.toUtf8();
    }
}

qint8 FrameDecoder::toInt8(const QByteArray &d, int o) const {
    return (o < d.size()) ? static_cast<qint8>(d[o]) : 0;
}
quint8 FrameDecoder::toUint8(const QByteArray &d, int o) const {
    return (o < d.size()) ? static_cast<quint8>(d[o]) : 0;
}
qint16 FrameDecoder::toInt16(const QByteArray &d, int o) const {
    if (o + 2 > d.size()) return 0;
    QByteArray slice = d.mid(o, 2);
    if (m_byteOrder == BigEndian) slice = swapBytes(slice);
    qint16 val; memcpy(&val, slice.constData(), 2); return val;
}
quint16 FrameDecoder::toUint16(const QByteArray &d, int o) const {
    return static_cast<quint16>(toInt16(d, o));
}
qint32 FrameDecoder::toInt32(const QByteArray &d, int o) const {
    if (o + 4 > d.size()) return 0;
    QByteArray slice = d.mid(o, 4);
    if (m_byteOrder == BigEndian) slice = swapBytes(slice);
    qint32 val; memcpy(&val, slice.constData(), 4); return val;
}
quint32 FrameDecoder::toUint32(const QByteArray &d, int o) const {
    return static_cast<quint32>(toInt32(d, o));
}
float FrameDecoder::toFloat(const QByteArray &d, int o) const {
    if (o + 4 > d.size()) return 0.0f;
    QByteArray slice = d.mid(o, 4);
    if (m_byteOrder == BigEndian) slice = swapBytes(slice);
    float val; memcpy(&val, slice.constData(), 4); return val;
}
double FrameDecoder::toDouble(const QByteArray &d, int o) const {
    if (o + 8 > d.size()) return 0.0;
    QByteArray slice = d.mid(o, 8);
    if (m_byteOrder == BigEndian) slice = swapBytes(slice);
    double val; memcpy(&val, slice.constData(), 8); return val;
}

QByteArray FrameDecoder::fromInt16(qint16 v) const {
    QByteArray r(2, '\0'); memcpy(r.data(), &v, 2);
    return (m_byteOrder == BigEndian) ? swapBytes(r) : r;
}
QByteArray FrameDecoder::fromInt32(qint32 v) const {
    QByteArray r(4, '\0'); memcpy(r.data(), &v, 4);
    return (m_byteOrder == BigEndian) ? swapBytes(r) : r;
}
QByteArray FrameDecoder::fromFloat(float v) const {
    QByteArray r(4, '\0'); memcpy(r.data(), &v, 4);
    return (m_byteOrder == BigEndian) ? swapBytes(r) : r;
}

FrameDecoder::Encoding FrameDecoder::encoding() const { return m_encoding; }
FrameDecoder::ByteOrder FrameDecoder::byteOrder() const { return m_byteOrder; }

QByteArray FrameDecoder::swapBytes(const QByteArray &d) const {
    QByteArray r(d.size(), '\0');
    for (int i = 0; i < d.size(); ++i) r[i] = d[d.size() - 1 - i];
    return r;
}
