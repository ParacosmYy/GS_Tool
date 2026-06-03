#pragma once
#include <QObject>
#include <QByteArray>
#include <QString>
#include <QMap>

class FrameDecoder : public QObject {
    Q_OBJECT
public:
    enum Encoding { Raw, Ascii, Hex, Base64, Utf8, Gb2312, Big5, ShiftJis };
    Q_ENUM(Encoding)

    enum ByteOrder { LittleEndian, BigEndian };
    Q_ENUM(ByteOrder)

    explicit FrameDecoder(QObject *parent = nullptr);
    ~FrameDecoder() override;

    void setEncoding(Encoding enc);
    void setByteOrder(ByteOrder order);

    QString decodeToString(const QByteArray &data) const;
    QByteArray encodeFromString(const QString &str) const;

    qint8  toInt8(const QByteArray &data, int offset = 0) const;
    quint8 toUint8(const QByteArray &data, int offset = 0) const;
    qint16 toInt16(const QByteArray &data, int offset = 0) const;
    quint16 toUint16(const QByteArray &data, int offset = 0) const;
    qint32 toInt32(const QByteArray &data, int offset = 0) const;
    quint32 toUint32(const QByteArray &data, int offset = 0) const;
    float  toFloat(const QByteArray &data, int offset = 0) const;
    double toDouble(const QByteArray &data, int offset = 0) const;

    QByteArray fromInt16(qint16 val) const;
    QByteArray fromInt32(qint32 val) const;
    QByteArray fromFloat(float val) const;

    Encoding encoding() const;
    ByteOrder byteOrder() const;

signals:
    void decoded(const QString &text);
    void decodeError(const QString &error);

private:
    QByteArray swapBytes(const QByteArray &data) const;
    Encoding m_encoding = Raw;
    ByteOrder m_byteOrder = LittleEndian;
};
