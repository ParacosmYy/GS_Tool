#include <QtTest/QtTest>

#include "apps/serial_station/core/SerialSession.h"

using namespace serial_station;

class SerialSessionTest : public QObject {
    Q_OBJECT

private slots:
    void invalidConfigWithoutPortName();
    void stateTransitionsClearErrors();
};

void SerialSessionTest::invalidConfigWithoutPortName()
{
    SerialPortConfig config;
    config.portName = QStringLiteral("  ");
    config.baudRate = 115200;

    QVERIFY(!config.isValid());
}

void SerialSessionTest::stateTransitionsClearErrors()
{
    SerialSession session;
    session.markOpening();
    QCOMPARE(session.state(), SerialSessionState::Opening);

    session.markOpen();
    QVERIFY(session.isOpen());
    QCOMPARE(session.errorString(), QString());

    session.markError(QStringLiteral("open failed"));
    QCOMPARE(session.state(), SerialSessionState::Error);
    QCOMPARE(session.errorString(), QStringLiteral("open failed"));

    session.markClosed();
    QCOMPARE(session.state(), SerialSessionState::Closed);
    QCOMPARE(session.errorString(), QString());
}

QTEST_MAIN(SerialSessionTest)
#include "test_serial_session.moc"
