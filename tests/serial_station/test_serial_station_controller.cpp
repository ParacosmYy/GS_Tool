#include <QtTest/QtTest>

#include <memory>

#include "apps/serial_station/SerialStationController.h"
#include "apps/serial_station/SerialStationConfig.h"
#include "apps/serial_station/core/SerialManager.h"

using serial_station::SerialPortConfig;
using serial_station::SerialSessionState;
using serial_station::SerialStationController;

class SerialStationControllerTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void defaultProtocolIsAvailable();
    void emptyCommandFailsBeforeFrameBuild_data();
    void emptyCommandFailsBeforeFrameBuild();
    void unsupportedModeFailsBeforeConnectionCheck_data();
    void unsupportedModeFailsBeforeConnectionCheck();
    void closedSerialPortRejectsAsciiSend();
    void closedSerialPortRejectsProtocolSend();
    void failureSignalsCarryNormalizedPayload();
    void rejectedCommandsDoNotEmitPortError();
    void rejectedCommandsKeepClosedSessionState();
    void repeatedRejectedCommandsCountEveryFailure();
    void emptyCommandSystemLogDoesNotInventCommandContext();
    void defaultProtocolBuildsAsciiFrame();
    void invalidConfigConnectReportsError();
    void disconnectReturnsClosedState();
};

void SerialStationControllerTest::initTestCase()
{
    qRegisterMetaType<SerialSessionState>("SerialSessionState");
}

void SerialStationControllerTest::defaultProtocolIsAvailable()
{
    SerialStationController controller;

    QVERIFY(controller.protocols().contains(QStringLiteral("ascii_text")));
    QCOMPARE(controller.protocols().defaultProtocol(), QStringLiteral("ascii_text"));
    QVERIFY(controller.protocols().createDefault() != nullptr);
}

void SerialStationControllerTest::emptyCommandFailsBeforeFrameBuild_data()
{
    QTest::addColumn<QString>("command");
    QTest::addColumn<QString>("mode");

    QTest::newRow("empty-ascii") << QString() << QStringLiteral("ascii");
    QTest::newRow("spaces-ascii") << QStringLiteral("   ") << QStringLiteral("ascii");
    QTest::newRow("tab-protocol") << QStringLiteral("\t\t") << QStringLiteral("protocol");
    QTest::newRow("newline-uppercase") << QStringLiteral("\n\r") << QStringLiteral("ASCII");
}

void SerialStationControllerTest::emptyCommandFailsBeforeFrameBuild()
{
    QFETCH(QString, command);
    QFETCH(QString, mode);

    SerialStationController controller;
    QSignalSpy failedSpy(&controller, &SerialStationController::serialCommandFailed);
    QSignalSpy preparedSpy(&controller, &SerialStationController::serialCommandPrepared);
    QSignalSpy txSpy(&controller, &SerialStationController::serialTxCounted);
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.sendCommand(command, mode);

    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(preparedSpy.count(), 0);
    QCOMPARE(txSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(systemLogSpy.count(), 1);
    QCOMPARE(failedSpy.takeFirst().at(2).toString(), QStringLiteral("发送内容为空"));
}

void SerialStationControllerTest::unsupportedModeFailsBeforeConnectionCheck_data()
{
    QTest::addColumn<QString>("command");
    QTest::addColumn<QString>("mode");
    QTest::addColumn<QString>("normalizedMode");

    QTest::newRow("hex-mode") << QStringLiteral("01 02") << QStringLiteral("hex") << QStringLiteral("hex");
    QTest::newRow("binary-mode") << QStringLiteral("PING") << QStringLiteral("binary") << QStringLiteral("binary");
    QTest::newRow("blank-mode") << QStringLiteral("PING") << QStringLiteral(" ") << QString();
    QTest::newRow("mixed-case") << QStringLiteral("PING") << QStringLiteral("Hex") << QStringLiteral("hex");
}

void SerialStationControllerTest::unsupportedModeFailsBeforeConnectionCheck()
{
    QFETCH(QString, command);
    QFETCH(QString, mode);
    QFETCH(QString, normalizedMode);

    SerialStationController controller;
    QSignalSpy failedSpy(&controller, &SerialStationController::serialCommandFailed);
    QSignalSpy preparedSpy(&controller, &SerialStationController::serialCommandPrepared);
    QSignalSpy txSpy(&controller, &SerialStationController::serialTxCounted);
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.sendCommand(command, mode);

    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(preparedSpy.count(), 0);
    QCOMPARE(txSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(systemLogSpy.count(), 1);

    const QList<QVariant> args = failedSpy.takeFirst();
    QCOMPARE(args.at(0).toString(), command.trimmed());
    QCOMPARE(args.at(1).toString(), normalizedMode);
    QVERIFY(args.at(2).toString().contains(QStringLiteral("发送模式暂不支持")));
}

void SerialStationControllerTest::closedSerialPortRejectsAsciiSend()
{
    SerialStationController controller;
    QSignalSpy failedSpy(&controller, &SerialStationController::serialCommandFailed);
    QSignalSpy preparedSpy(&controller, &SerialStationController::serialCommandPrepared);
    QSignalSpy sentSpy(&controller, &SerialStationController::serialCommandSent);
    QSignalSpy txLogSpy(&controller, &SerialStationController::serialTxLogged);
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorCounted);

    controller.sendCommand(QStringLiteral(" AT+GMR "), QStringLiteral("ASCII"));

    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(preparedSpy.count(), 0);
    QCOMPARE(sentSpy.count(), 0);
    QCOMPARE(txLogSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);

    const QList<QVariant> args = failedSpy.takeFirst();
    QCOMPARE(args.at(0).toString(), QStringLiteral("AT+GMR"));
    QCOMPARE(args.at(1).toString(), QStringLiteral("ascii"));
    QCOMPARE(args.at(2).toString(), QStringLiteral("串口未连接，无法发送"));
}

void SerialStationControllerTest::closedSerialPortRejectsProtocolSend()
{
    SerialStationController controller;
    QSignalSpy failedSpy(&controller, &SerialStationController::serialCommandFailed);
    QSignalSpy preparedSpy(&controller, &SerialStationController::serialCommandPrepared);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.sendCommand(QStringLiteral("PING"), QStringLiteral("protocol"));

    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(preparedSpy.count(), 0);
    QCOMPARE(systemLogSpy.count(), 1);
    QVERIFY(systemLogSpy.takeFirst().at(0).toString().contains(QStringLiteral("PING")));
}

void SerialStationControllerTest::failureSignalsCarryNormalizedPayload()
{
    SerialStationController controller;
    QSignalSpy failedSpy(&controller, &SerialStationController::serialCommandFailed);
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.sendCommand(QStringLiteral("  RESET  "), QStringLiteral("  ASCII  "));

    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(systemLogSpy.count(), 1);

    const QList<QVariant> args = failedSpy.takeFirst();
    QCOMPARE(args.at(0).toString(), QStringLiteral("RESET"));
    QCOMPARE(args.at(1).toString(), QStringLiteral("ascii"));
    QCOMPARE(args.at(2).toString(), QStringLiteral("串口未连接，无法发送"));
    QVERIFY(systemLogSpy.takeFirst().at(0).toString().contains(QStringLiteral("RESET")));
}

void SerialStationControllerTest::rejectedCommandsDoNotEmitPortError()
{
    SerialStationController controller;
    QSignalSpy portErrorSpy(&controller, &SerialStationController::serialErrorOccurred);
    QSignalSpy commandFailedSpy(&controller, &SerialStationController::serialCommandFailed);

    controller.sendCommand(QString(), QStringLiteral("ascii"));
    controller.sendCommand(QStringLiteral("PING"), QStringLiteral("hex"));
    controller.sendCommand(QStringLiteral("PING"), QStringLiteral("ascii"));

    QCOMPARE(commandFailedSpy.count(), 3);
    QCOMPARE(portErrorSpy.count(), 0);
}

void SerialStationControllerTest::rejectedCommandsKeepClosedSessionState()
{
    SerialStationController controller;

    QCOMPARE(controller.serialManager().session().state(), SerialSessionState::Closed);
    controller.sendCommand(QStringLiteral("PING"), QStringLiteral("ascii"));
    QCOMPARE(controller.serialManager().session().state(), SerialSessionState::Closed);
    controller.sendCommand(QStringLiteral("PING"), QStringLiteral("protocol"));
    QCOMPARE(controller.serialManager().session().state(), SerialSessionState::Closed);
    controller.sendCommand(QStringLiteral("PING"), QStringLiteral("hex"));
    QCOMPARE(controller.serialManager().session().state(), SerialSessionState::Closed);
}

void SerialStationControllerTest::repeatedRejectedCommandsCountEveryFailure()
{
    SerialStationController controller;
    QSignalSpy errorCountSpy(&controller, &SerialStationController::serialErrorCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);
    QSignalSpy failedSpy(&controller, &SerialStationController::serialCommandFailed);

    controller.sendCommand(QStringLiteral("AT+GMR"), QStringLiteral("ascii"));
    controller.sendCommand(QStringLiteral("PING"), QStringLiteral("protocol"));
    controller.sendCommand(QStringLiteral("01 02"), QStringLiteral("hex"));
    controller.sendCommand(QStringLiteral("RESET"), QStringLiteral("binary"));

    QCOMPARE(errorCountSpy.count(), 4);
    QCOMPARE(systemLogSpy.count(), 4);
    QCOMPARE(failedSpy.count(), 4);
    QVERIFY(systemLogSpy.at(0).at(0).toString().contains(QStringLiteral("AT+GMR")));
    QVERIFY(systemLogSpy.at(1).at(0).toString().contains(QStringLiteral("PING")));
    QVERIFY(systemLogSpy.at(2).at(0).toString().contains(QStringLiteral("01 02")));
    QVERIFY(systemLogSpy.at(3).at(0).toString().contains(QStringLiteral("RESET")));
}

void SerialStationControllerTest::emptyCommandSystemLogDoesNotInventCommandContext()
{
    SerialStationController controller;
    QSignalSpy failedSpy(&controller, &SerialStationController::serialCommandFailed);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.sendCommand(QStringLiteral("   "), QStringLiteral("ascii"));

    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(systemLogSpy.count(), 1);

    const QString failedMessage = failedSpy.takeFirst().at(2).toString();
    const QString systemMessage = systemLogSpy.takeFirst().at(0).toString();
    QCOMPARE(failedMessage, QStringLiteral("发送内容为空"));
    QCOMPARE(systemMessage, QStringLiteral("发送内容为空"));
    QVERIFY(!systemMessage.contains(QStringLiteral(":")));
}

void SerialStationControllerTest::defaultProtocolBuildsAsciiFrame()
{
    SerialStationController controller;
    const std::unique_ptr<serial_station::ISerialProtocol> protocol =
        controller.protocols().createDefault();
    QVERIFY(protocol != nullptr);

    QVariantMap params;
    params.insert(QStringLiteral("text"), QStringLiteral("AT+GMR"));
    params.insert(QStringLiteral("appendNewline"), false);
    QCOMPARE(protocol->buildCommand(QStringLiteral("ignored"), params), QByteArray("AT+GMR"));

    params.insert(QStringLiteral("appendNewline"), true);
    QCOMPARE(protocol->buildCommand(QStringLiteral("ignored"), params), QByteArray("AT+GMR\n"));
}

void SerialStationControllerTest::invalidConfigConnectReportsError()
{
    SerialStationController controller;
    SerialPortConfig invalidConfig;
    invalidConfig.portName.clear();
    invalidConfig.baudRate = 115200;

    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorOccurred);
    QSignalSpy stateSpy(&controller, &SerialStationController::serialStateChanged);

    controller.connectSerialPort(invalidConfig);

    QVERIFY(errorSpy.count() >= 1);
    QVERIFY(stateSpy.count() >= 1);
    QCOMPARE(controller.serialManager().session().state(), SerialSessionState::Error);
    QVERIFY(!controller.serialManager().session().isOpen());
}

void SerialStationControllerTest::disconnectReturnsClosedState()
{
    SerialStationController controller;
    QSignalSpy stateSpy(&controller, &SerialStationController::serialStateChanged);

    controller.disconnectSerialPort();

    QCOMPARE(controller.serialManager().session().state(), SerialSessionState::Closed);
    QCOMPARE(stateSpy.count(), 1);
}

QTEST_MAIN(SerialStationControllerTest)
#include "test_serial_station_controller.moc"
