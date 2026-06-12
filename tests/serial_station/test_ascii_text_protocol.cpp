#include <QtTest/QtTest>

#include "apps/serial_station/protocols/ascii_text/AsciiTextProtocol.h"

using namespace serial_station;

class AsciiTextProtocolTest : public QObject {
    Q_OBJECT

private slots:
    void buildCommandUsesTextParam();
    void buildCommandCanAppendNewline();
    void feedEmitsCompleteLinesOnly();
    void resetClearsPartialLine();
};

void AsciiTextProtocolTest::buildCommandUsesTextParam()
{
    AsciiTextProtocol protocol;
    QVariantMap params;
    params.insert(QStringLiteral("text"), QStringLiteral("AT+RST"));

    QCOMPARE(protocol.buildCommand(QString(), params), QByteArray("AT+RST"));
}

void AsciiTextProtocolTest::buildCommandCanAppendNewline()
{
    AsciiTextProtocol protocol;
    QVariantMap params;
    params.insert(QStringLiteral("text"), QStringLiteral("PING"));
    params.insert(QStringLiteral("appendNewline"), true);

    QCOMPARE(protocol.buildCommand(QString(), params), QByteArray("PING\n"));
}

void AsciiTextProtocolTest::feedEmitsCompleteLinesOnly()
{
    AsciiTextProtocol protocol;

    QVector<SerialProtocolEvent> first = protocol.feed(QByteArray("hello"));
    QVERIFY(first.isEmpty());

    QVector<SerialProtocolEvent> second = protocol.feed(QByteArray(" world\r\nnext\npartial"));
    QCOMPARE(second.size(), 2);
    QCOMPARE(second.at(0).payload.value(QStringLiteral("text")).toString(), QStringLiteral("hello world"));
    QCOMPARE(second.at(1).payload.value(QStringLiteral("text")).toString(), QStringLiteral("next"));
    QCOMPARE(second.at(0).raw, QByteArray("hello world\r\n"));
}

void AsciiTextProtocolTest::resetClearsPartialLine()
{
    AsciiTextProtocol protocol;
    protocol.feed(QByteArray("partial"));
    protocol.reset();

    QVector<SerialProtocolEvent> events = protocol.feed(QByteArray("done\n"));
    QCOMPARE(events.size(), 1);
    QCOMPARE(events.first().payload.value(QStringLiteral("text")).toString(), QStringLiteral("done"));
}

QTEST_MAIN(AsciiTextProtocolTest)
#include "test_ascii_text_protocol.moc"
