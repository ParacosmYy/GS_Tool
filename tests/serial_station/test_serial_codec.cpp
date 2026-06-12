#include <QtTest/QtTest>

#include "apps/serial_station/core/SerialCodec.h"
#include "apps/serial_station/protocols/ascii_text/AsciiTextProtocol.h"

using serial_station::AsciiTextProtocol;
using serial_station::SerialCodec;

class SerialCodecTest : public QObject {
    Q_OBJECT

private slots:
    void normalizeModeTrimsAndLowercases_data();
    void normalizeModeTrimsAndLowercases();
    void asciiEncodesUtf8Text();
    void asciiTrimsBeforeEncoding();
    void hexEncodesCommonFormats_data();
    void hexEncodesCommonFormats();
    void hexRejectsInvalidInput_data();
    void hexRejectsInvalidInput();
    void protocolDelegatesToDefaultProtocol();
    void protocolRejectsMissingProtocol();
    void unknownModeReturnsActionableError_data();
    void unknownModeReturnsActionableError();
    void emptyCommandRejectedBeforeModeSpecificWork_data();
    void emptyCommandRejectedBeforeModeSpecificWork();
};

void SerialCodecTest::normalizeModeTrimsAndLowercases_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("ascii") << QStringLiteral("ASCII") << QStringLiteral("ascii");
    QTest::newRow("hex") << QStringLiteral(" Hex ") << QStringLiteral("hex");
    QTest::newRow("protocol") << QStringLiteral("\tPROTOCOL\r\n") << QStringLiteral("protocol");
    QTest::newRow("blank") << QStringLiteral("   ") << QString();
}

void SerialCodecTest::normalizeModeTrimsAndLowercases()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);

    const SerialCodec codec;

    QCOMPARE(codec.normalizeMode(input), expected);
}

void SerialCodecTest::asciiEncodesUtf8Text()
{
    const SerialCodec codec;

    const SerialCodec::EncodeResult result =
        codec.encode(QStringLiteral("AT+GMR"), QStringLiteral("ascii"), nullptr);

    QVERIFY(result.ok);
    QCOMPARE(result.normalizedMode, QStringLiteral("ascii"));
    QCOMPARE(result.frame, QByteArray("AT+GMR"));
    QVERIFY(result.errorMessage.isEmpty());
}

void SerialCodecTest::asciiTrimsBeforeEncoding()
{
    const SerialCodec codec;

    const SerialCodec::EncodeResult result =
        codec.encode(QStringLiteral("  PING  "), QStringLiteral(" ASCII "), nullptr);

    QVERIFY(result.ok);
    QCOMPARE(result.normalizedMode, QStringLiteral("ascii"));
    QCOMPARE(result.frame, QByteArray("PING"));
}

void SerialCodecTest::hexEncodesCommonFormats_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QByteArray>("expected");

    QTest::newRow("space") << QStringLiteral("01 03 00 00 00 02")
                           << QByteArray::fromHex("010300000002");
    QTest::newRow("compact") << QStringLiteral("010300000002")
                             << QByteArray::fromHex("010300000002");
    QTest::newRow("comma") << QStringLiteral("01,03,00,00,00,02")
                           << QByteArray::fromHex("010300000002");
    QTest::newRow("prefix") << QStringLiteral("0x01 0x03 0x00 0x02")
                            << QByteArray::fromHex("01030002");
    QTest::newRow("newline") << QStringLiteral("01\n03\r\n00\t02")
                             << QByteArray::fromHex("01030002");
    QTest::newRow("lowercase") << QStringLiteral("aa bb cc dd")
                               << QByteArray::fromHex("aabbccdd");
}

void SerialCodecTest::hexEncodesCommonFormats()
{
    QFETCH(QString, input);
    QFETCH(QByteArray, expected);

    const SerialCodec codec;

    const SerialCodec::EncodeResult result =
        codec.encode(input, QStringLiteral("hex"), nullptr);

    QVERIFY(result.ok);
    QCOMPARE(result.normalizedMode, QStringLiteral("hex"));
    QCOMPARE(result.frame, expected);
    QVERIFY(result.errorMessage.isEmpty());
}

void SerialCodecTest::hexRejectsInvalidInput_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("message");

    QTest::newRow("odd") << QStringLiteral("0") << QStringLiteral("HEX 字符数量必须为偶数");
    QTest::newRow("bad-char") << QStringLiteral("01 GG")
                              << QStringLiteral("HEX 内容包含非法字符");
    QTest::newRow("only-prefix") << QStringLiteral("0x")
                                 << QStringLiteral("HEX 内容为空");
    QTest::newRow("separators") << QStringLiteral(" , \n\t ")
                                << QStringLiteral("HEX 内容为空");
}

void SerialCodecTest::hexRejectsInvalidInput()
{
    QFETCH(QString, input);
    QFETCH(QString, message);

    const SerialCodec codec;

    const SerialCodec::EncodeResult result =
        codec.encode(input, QStringLiteral("hex"), nullptr);

    QVERIFY(!result.ok);
    QCOMPARE(result.normalizedMode, QStringLiteral("hex"));
    QVERIFY(result.frame.isEmpty());
    QCOMPARE(result.errorMessage, message);
}

void SerialCodecTest::protocolDelegatesToDefaultProtocol()
{
    const SerialCodec codec;
    const AsciiTextProtocol protocol;

    const SerialCodec::EncodeResult result =
        codec.encode(QStringLiteral("READ_ID"), QStringLiteral("protocol"), &protocol);

    QVERIFY(result.ok);
    QCOMPARE(result.normalizedMode, QStringLiteral("protocol"));
    QCOMPARE(result.frame, QByteArray("READ_ID"));
}

void SerialCodecTest::protocolRejectsMissingProtocol()
{
    const SerialCodec codec;

    const SerialCodec::EncodeResult result =
        codec.encode(QStringLiteral("READ_ID"), QStringLiteral("protocol"), nullptr);

    QVERIFY(!result.ok);
    QCOMPARE(result.normalizedMode, QStringLiteral("protocol"));
    QCOMPARE(result.errorMessage, QStringLiteral("默认协议不可用"));
}

void SerialCodecTest::unknownModeReturnsActionableError_data()
{
    QTest::addColumn<QString>("mode");
    QTest::addColumn<QString>("normalizedMode");

    QTest::newRow("binary") << QStringLiteral("binary") << QStringLiteral("binary");
    QTest::newRow("blank") << QStringLiteral(" ") << QString();
    QTest::newRow("mixed") << QStringLiteral(" Raw ") << QStringLiteral("raw");
}

void SerialCodecTest::unknownModeReturnsActionableError()
{
    QFETCH(QString, mode);
    QFETCH(QString, normalizedMode);

    const SerialCodec codec;

    const SerialCodec::EncodeResult result =
        codec.encode(QStringLiteral("PING"), mode, nullptr);

    QVERIFY(!result.ok);
    QCOMPARE(result.normalizedMode, normalizedMode);
    QVERIFY(result.errorMessage.contains(QStringLiteral("发送模式暂不支持")));
}

void SerialCodecTest::emptyCommandRejectedBeforeModeSpecificWork_data()
{
    QTest::addColumn<QString>("command");
    QTest::addColumn<QString>("mode");
    QTest::addColumn<QString>("normalizedMode");

    QTest::newRow("empty-ascii") << QString() << QStringLiteral("ascii") << QStringLiteral("ascii");
    QTest::newRow("spaces-hex") << QStringLiteral("   ") << QStringLiteral("hex") << QStringLiteral("hex");
    QTest::newRow("tabs-protocol") << QStringLiteral("\t\t") << QStringLiteral("protocol")
                                   << QStringLiteral("protocol");
}

void SerialCodecTest::emptyCommandRejectedBeforeModeSpecificWork()
{
    QFETCH(QString, command);
    QFETCH(QString, mode);
    QFETCH(QString, normalizedMode);

    const SerialCodec codec;

    const SerialCodec::EncodeResult result = codec.encode(command, mode, nullptr);

    QVERIFY(!result.ok);
    QCOMPARE(result.normalizedMode, normalizedMode);
    QCOMPARE(result.errorMessage, QStringLiteral("发送内容为空"));
}

QTEST_MAIN(SerialCodecTest)
#include "test_serial_codec.moc"
