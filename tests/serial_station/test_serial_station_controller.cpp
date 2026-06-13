#include <QtTest/QtTest>

#include <QtCore/QDataStream>
#include <QtCore/QIODevice>

#include <memory>

#include "apps/serial_station/SerialStationController.h"
#include "apps/serial_station/SerialStationConfig.h"
#include "apps/serial_station/core/SerialManager.h"

using serial_station::SerialPortConfig;
using serial_station::SerialSessionState;
using serial_station::SerialStationController;

Q_DECLARE_METATYPE(SerialPortConfig)

namespace {

QByteArray controllerTestFloatBytes(float value)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream << value;
    return bytes;
}

QByteArray controllerTestJustFloatFrame(std::initializer_list<float> values)
{
    QByteArray frame;
    for (const float value : values) {
        frame.append(controllerTestFloatBytes(value));
    }
    frame.append(QByteArray::fromHex("0000807F"));
    return frame;
}

} // namespace

class SerialStationControllerTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void defaultProtocolIsAvailable();
    void availableProtocolNamesExposeBuiltIns();
    void activeProtocolSwitchUpdatesDefaultAndLogs();
    void missingActiveProtocolDoesNotChangeDefault();
    void selectingSameProtocolDoesNotCreateSystemLog();
    void emptyCommandFailsBeforeFrameBuild_data();
    void emptyCommandFailsBeforeFrameBuild();
    void unsupportedModeFailsBeforeConnectionCheck_data();
    void unsupportedModeFailsBeforeConnectionCheck();
    void closedSerialPortRejectsAsciiSend();
    void closedSerialPortRejectsHexSend();
    void invalidHexFailsBeforeConnectionCheck_data();
    void invalidHexFailsBeforeConnectionCheck();
    void closedSerialPortRejectsProtocolSend();
    void failureSignalsCarryNormalizedPayload();
    void rejectedCommandsDoNotEmitPortError();
    void rejectedCommandsKeepClosedSessionState();
    void repeatedRejectedCommandsCountEveryFailure();
    void emptyCommandSystemLogDoesNotInventCommandContext();
    void defaultProtocolBuildsAsciiFrame();
    void emptyReceiveBytesAreIgnored();
    void completeAsciiLineProducesRxLog();
    void completeAsciiLineDoesNotCountErrors();
    void splitAsciiLineBuffersUntilNewline();
    void multipleAsciiLinesProduceMultipleRxCounts();
    void carriageReturnIsTrimmedFromAsciiLine();
    void partialReceiveBytesCreateSystemCacheLog();
    void partialReceiveThenCompleteLineKeepsBufferedContent();
    void emptyAsciiLineFallsBackToRawSummary();
    void newlineOnlyAfterPartialProducesBufferedFrame();
    void oversizedPartialBufferIsClearedByProtocol();
    void receiveAfterOversizedBufferStartsFresh();
    void emptyBytesBetweenPartialChunksDoNotBreakFrame();
    void justFloatMeasurementProducesRxLog();
    void justFloatMeasurementEmitsSummaryLines();
    void clearLogRecordsClearsMeasurementSummary();
    void reconnectResetsPartialReceiveBuffer();
    void disconnectResetsPartialReceiveBuffer();
    void disconnectThenNewLineDoesNotUseOldPartialBytes();
    void validConfigConnectLogsConfigSummaryBeforeOpen();
    void invalidConfigConnectReportsError();
    void invalidConfigConnectReportsConfigValidationReason_data();
    void invalidConfigConnectReportsConfigValidationReason();
    void disconnectReturnsClosedState();
};

void SerialStationControllerTest::initTestCase()
{
    qRegisterMetaType<SerialSessionState>("SerialSessionState");
    qRegisterMetaType<SerialPortConfig>("SerialPortConfig");
}

void SerialStationControllerTest::defaultProtocolIsAvailable()
{
    SerialStationController controller;

    QVERIFY(controller.protocols().contains(QStringLiteral("ascii_text")));
    QCOMPARE(controller.protocols().defaultProtocol(), QStringLiteral("ascii_text"));
    QVERIFY(controller.protocols().createDefault() != nullptr);
}

void SerialStationControllerTest::availableProtocolNamesExposeBuiltIns()
{
    SerialStationController controller;
    const QStringList names = controller.availableProtocolNames();

    QVERIFY(names.contains(QStringLiteral("ascii_text")));
    QVERIFY(names.contains(QStringLiteral("custom_md")));
    QVERIFY(names.contains(QStringLiteral("modbus_rtu")));
    QCOMPARE(controller.activeProtocolName(), QStringLiteral("ascii_text"));
}

void SerialStationControllerTest::activeProtocolSwitchUpdatesDefaultAndLogs()
{
    SerialStationController controller;
    QSignalSpy changedSpy(&controller, &SerialStationController::activeProtocolChanged);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorCounted);

    controller.setActiveProtocol(QStringLiteral("custom_md"));

    QCOMPARE(controller.activeProtocolName(), QStringLiteral("custom_md"));
    QCOMPARE(controller.protocols().defaultProtocol(), QStringLiteral("custom_md"));
    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(systemLogSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(changedSpy.takeFirst().at(0).toString(), QStringLiteral("custom_md"));
    QVERIFY(systemLogSpy.takeFirst().at(0).toString().contains(QStringLiteral("custom_md")));
}

void SerialStationControllerTest::missingActiveProtocolDoesNotChangeDefault()
{
    SerialStationController controller;
    QSignalSpy changedSpy(&controller, &SerialStationController::activeProtocolChanged);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorCounted);

    controller.setActiveProtocol(QStringLiteral("custom_md"));
    controller.setActiveProtocol(QStringLiteral("missing_protocol"));

    QCOMPARE(controller.activeProtocolName(), QStringLiteral("custom_md"));
    QCOMPARE(changedSpy.count(), 2);
    QCOMPARE(systemLogSpy.count(), 2);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(changedSpy.takeLast().at(0).toString(), QStringLiteral("custom_md"));
    QVERIFY(systemLogSpy.takeLast().at(0).toString().contains(QStringLiteral("missing_protocol")));
}

void SerialStationControllerTest::selectingSameProtocolDoesNotCreateSystemLog()
{
    SerialStationController controller;
    QSignalSpy changedSpy(&controller, &SerialStationController::activeProtocolChanged);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.setActiveProtocol(QStringLiteral("ascii_text"));

    QCOMPARE(controller.activeProtocolName(), QStringLiteral("ascii_text"));
    QCOMPARE(changedSpy.count(), 1);
    QCOMPARE(systemLogSpy.count(), 0);
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

    QTest::newRow("binary-mode") << QStringLiteral("PING") << QStringLiteral("binary") << QStringLiteral("binary");
    QTest::newRow("blank-mode") << QStringLiteral("PING") << QStringLiteral(" ") << QString();
    QTest::newRow("raw-mode") << QStringLiteral("PING") << QStringLiteral("Raw") << QStringLiteral("raw");
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

void SerialStationControllerTest::closedSerialPortRejectsHexSend()
{
    SerialStationController controller;
    QSignalSpy failedSpy(&controller, &SerialStationController::serialCommandFailed);
    QSignalSpy preparedSpy(&controller, &SerialStationController::serialCommandPrepared);
    QSignalSpy sentSpy(&controller, &SerialStationController::serialCommandSent);
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorCounted);

    controller.sendCommand(QStringLiteral(" 01 03 00 00 00 02 "), QStringLiteral(" HEX "));

    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(preparedSpy.count(), 0);
    QCOMPARE(sentSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);

    const QList<QVariant> args = failedSpy.takeFirst();
    QCOMPARE(args.at(0).toString(), QStringLiteral("01 03 00 00 00 02"));
    QCOMPARE(args.at(1).toString(), QStringLiteral("hex"));
    QCOMPARE(args.at(2).toString(), QStringLiteral("串口未连接，无法发送"));
}

void SerialStationControllerTest::invalidHexFailsBeforeConnectionCheck_data()
{
    QTest::addColumn<QString>("command");
    QTest::addColumn<QString>("message");

    QTest::newRow("odd") << QStringLiteral("0") << QStringLiteral("HEX 字符数量必须为偶数");
    QTest::newRow("bad-char") << QStringLiteral("01 ZZ") << QStringLiteral("HEX 内容包含非法字符");
    QTest::newRow("empty-after-prefix") << QStringLiteral("0x") << QStringLiteral("HEX 内容为空");
}

void SerialStationControllerTest::invalidHexFailsBeforeConnectionCheck()
{
    QFETCH(QString, command);
    QFETCH(QString, message);

    SerialStationController controller;
    QSignalSpy failedSpy(&controller, &SerialStationController::serialCommandFailed);
    QSignalSpy preparedSpy(&controller, &SerialStationController::serialCommandPrepared);
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.sendCommand(command, QStringLiteral("hex"));

    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(preparedSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(systemLogSpy.count(), 1);

    const QList<QVariant> args = failedSpy.takeFirst();
    QCOMPARE(args.at(0).toString(), command.trimmed());
    QCOMPARE(args.at(1).toString(), QStringLiteral("hex"));
    QCOMPARE(args.at(2).toString(), message);
    QVERIFY(systemLogSpy.takeFirst().at(0).toString().contains(message));
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

void SerialStationControllerTest::emptyReceiveBytesAreIgnored()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorCounted);

    controller.handleBytesReceived(QByteArray());

    QCOMPARE(rxSpy.count(), 0);
    QCOMPARE(rxCountSpy.count(), 0);
    QCOMPARE(systemLogSpy.count(), 0);
    QCOMPARE(errorSpy.count(), 0);
}

void SerialStationControllerTest::completeAsciiLineProducesRxLog()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.handleBytesReceived(QByteArray("OK\n"));

    QCOMPARE(rxSpy.count(), 1);
    QCOMPARE(rxCountSpy.count(), 1);
    QCOMPARE(systemLogSpy.count(), 0);
    QCOMPARE(rxSpy.takeFirst().at(0).toString(), QStringLiteral("OK"));
}

void SerialStationControllerTest::completeAsciiLineDoesNotCountErrors()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.handleBytesReceived(QByteArray("READY\n"));

    QCOMPARE(rxSpy.count(), 1);
    QCOMPARE(rxCountSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    QCOMPARE(systemLogSpy.count(), 0);
}

void SerialStationControllerTest::splitAsciiLineBuffersUntilNewline()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.handleBytesReceived(QByteArray("O"));

    QCOMPARE(rxSpy.count(), 0);
    QCOMPARE(rxCountSpy.count(), 0);
    QCOMPARE(systemLogSpy.count(), 1);
    QVERIFY(systemLogSpy.takeFirst().at(0).toString().contains(QStringLiteral("接收缓存")));

    controller.handleBytesReceived(QByteArray("K\n"));

    QCOMPARE(rxSpy.count(), 1);
    QCOMPARE(rxCountSpy.count(), 1);
    QCOMPARE(rxSpy.takeFirst().at(0).toString(), QStringLiteral("OK"));
}

void SerialStationControllerTest::multipleAsciiLinesProduceMultipleRxCounts()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.handleBytesReceived(QByteArray("OK\nERR\nREADY\n"));

    QCOMPARE(rxSpy.count(), 3);
    QCOMPARE(rxCountSpy.count(), 3);
    QCOMPARE(systemLogSpy.count(), 0);
    QCOMPARE(rxSpy.at(0).at(0).toString(), QStringLiteral("OK"));
    QCOMPARE(rxSpy.at(1).at(0).toString(), QStringLiteral("ERR"));
    QCOMPARE(rxSpy.at(2).at(0).toString(), QStringLiteral("READY"));
}

void SerialStationControllerTest::carriageReturnIsTrimmedFromAsciiLine()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);

    controller.handleBytesReceived(QByteArray("VERSION 1.0\r\n"));

    QCOMPARE(rxSpy.count(), 1);
    QCOMPARE(rxCountSpy.count(), 1);
    QCOMPARE(rxSpy.takeFirst().at(0).toString(), QStringLiteral("VERSION 1.0"));
}

void SerialStationControllerTest::partialReceiveBytesCreateSystemCacheLog()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorCounted);

    controller.handleBytesReceived(QByteArray("PARTIAL"));

    QCOMPARE(rxSpy.count(), 0);
    QCOMPARE(rxCountSpy.count(), 0);
    QCOMPARE(systemLogSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    QVERIFY(systemLogSpy.takeFirst().at(0).toString().contains(QStringLiteral("7 bytes")));
}

void SerialStationControllerTest::partialReceiveThenCompleteLineKeepsBufferedContent()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.handleBytesReceived(QByteArray("VER"));
    controller.handleBytesReceived(QByteArray("SION"));
    controller.handleBytesReceived(QByteArray("?\n"));

    QCOMPARE(rxSpy.count(), 1);
    QCOMPARE(rxCountSpy.count(), 1);
    QCOMPARE(systemLogSpy.count(), 2);
    QCOMPARE(rxSpy.takeFirst().at(0).toString(), QStringLiteral("VERSION?"));
}

void SerialStationControllerTest::emptyAsciiLineFallsBackToRawSummary()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.handleBytesReceived(QByteArray("\n"));

    QCOMPARE(rxSpy.count(), 1);
    QCOMPARE(rxCountSpy.count(), 1);
    QCOMPARE(systemLogSpy.count(), 0);
    QCOMPARE(rxSpy.takeFirst().at(0).toString(), QStringLiteral("0A"));
}

void SerialStationControllerTest::newlineOnlyAfterPartialProducesBufferedFrame()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.handleBytesReceived(QByteArray("ACK"));
    controller.handleBytesReceived(QByteArray("\n"));

    QCOMPARE(rxSpy.count(), 1);
    QCOMPARE(rxCountSpy.count(), 1);
    QCOMPARE(systemLogSpy.count(), 1);
    QCOMPARE(rxSpy.takeFirst().at(0).toString(), QStringLiteral("ACK"));
}

void SerialStationControllerTest::oversizedPartialBufferIsClearedByProtocol()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);
    const QByteArray oversized(9000, 'A');

    controller.handleBytesReceived(oversized);
    controller.handleBytesReceived(QByteArray("\n"));

    QCOMPARE(rxSpy.count(), 1);
    QCOMPARE(rxCountSpy.count(), 1);
    QVERIFY(systemLogSpy.count() >= 1);
    QCOMPARE(rxSpy.takeFirst().at(0).toString(), QStringLiteral("0A"));
}

void SerialStationControllerTest::receiveAfterOversizedBufferStartsFresh()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);
    const QByteArray oversized(9000, 'B');

    controller.handleBytesReceived(oversized);
    controller.handleBytesReceived(QByteArray("OK\n"));

    QCOMPARE(rxSpy.count(), 1);
    QCOMPARE(rxCountSpy.count(), 1);
    QVERIFY(systemLogSpy.count() >= 1);
    QCOMPARE(rxSpy.takeFirst().at(0).toString(), QStringLiteral("OK"));
}

void SerialStationControllerTest::emptyBytesBetweenPartialChunksDoNotBreakFrame()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.handleBytesReceived(QByteArray("O"));
    controller.handleBytesReceived(QByteArray());
    controller.handleBytesReceived(QByteArray("K\n"));

    QCOMPARE(rxSpy.count(), 1);
    QCOMPARE(rxCountSpy.count(), 1);
    QCOMPARE(systemLogSpy.count(), 1);
    QCOMPARE(rxSpy.takeFirst().at(0).toString(), QStringLiteral("OK"));
}

void SerialStationControllerTest::justFloatMeasurementProducesRxLog()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorCounted);

    controller.setActiveProtocol(QStringLiteral("just_float"));
    controller.handleBytesReceived(controllerTestJustFloatFrame({1.5F, -2.25F}));

    QCOMPARE(rxSpy.count(), 1);
    QCOMPARE(rxCountSpy.count(), 1);
    QCOMPARE(errorSpy.count(), 0);
    const QString text = rxSpy.takeFirst().at(0).toString();
    QVERIFY(text.contains(QStringLiteral("JustFloat")));
    QVERIFY(text.contains(QStringLiteral("1.5")));
    QVERIFY(text.contains(QStringLiteral("-2.25")));
}

void SerialStationControllerTest::justFloatMeasurementEmitsSummaryLines()
{
    SerialStationController controller;
    QSignalSpy measurementSpy(&controller,
                              &SerialStationController::serialMeasurementUpdated);

    controller.setActiveProtocol(QStringLiteral("just_float"));
    controller.handleBytesReceived(controllerTestJustFloatFrame({1.5F, -2.25F}));

    QCOMPARE(measurementSpy.count(), 1);
    const QStringList lines = measurementSpy.takeFirst().at(0).toStringList();
    QCOMPARE(lines.size(), 2);
    QVERIFY(lines.at(0).contains(QStringLiteral("ch1")));
    QVERIFY(lines.at(0).contains(QStringLiteral("1.5")));
    QVERIFY(lines.at(1).contains(QStringLiteral("ch2")));
    QVERIFY(lines.at(1).contains(QStringLiteral("-2.25")));
}

void SerialStationControllerTest::clearLogRecordsClearsMeasurementSummary()
{
    SerialStationController controller;
    QSignalSpy measurementSpy(&controller,
                              &SerialStationController::serialMeasurementUpdated);

    controller.setActiveProtocol(QStringLiteral("just_float"));
    controller.handleBytesReceived(controllerTestJustFloatFrame({1.0F}));
    controller.clearLogRecords();

    QCOMPARE(measurementSpy.count(), 2);
    QVERIFY(measurementSpy.takeLast().at(0).toStringList().isEmpty());
}

void SerialStationControllerTest::reconnectResetsPartialReceiveBuffer()
{
    SerialStationController controller;
    SerialPortConfig invalidConfig;
    invalidConfig.portName.clear();
    invalidConfig.baudRate = 115200;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.handleBytesReceived(QByteArray("STALE"));
    controller.connectSerialPort(invalidConfig);
    controller.handleBytesReceived(QByteArray("\n"));

    QCOMPARE(rxSpy.count(), 1);
    QVERIFY(systemLogSpy.count() >= 1);
    QCOMPARE(rxSpy.takeFirst().at(0).toString(), QStringLiteral("0A"));
}

void SerialStationControllerTest::disconnectResetsPartialReceiveBuffer()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.handleBytesReceived(QByteArray("STALE"));
    controller.disconnectSerialPort();
    controller.handleBytesReceived(QByteArray("\n"));

    QCOMPARE(rxSpy.count(), 1);
    QVERIFY(systemLogSpy.count() >= 1);
    QCOMPARE(rxSpy.takeFirst().at(0).toString(), QStringLiteral("0A"));
}

void SerialStationControllerTest::disconnectThenNewLineDoesNotUseOldPartialBytes()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);

    controller.handleBytesReceived(QByteArray("OLD"));
    controller.disconnectSerialPort();
    controller.handleBytesReceived(QByteArray("NEW\n"));

    QCOMPARE(rxSpy.count(), 1);
    QCOMPARE(rxCountSpy.count(), 1);
    QCOMPARE(rxSpy.takeFirst().at(0).toString(), QStringLiteral("NEW"));
}

void SerialStationControllerTest::validConfigConnectLogsConfigSummaryBeforeOpen()
{
    SerialStationController controller;
    SerialPortConfig config;
    config.portName = QStringLiteral("  COM_DOES_NOT_EXIST  ");
    config.baudRate = 57600;
    config.dataBits = QSerialPort::Data7;
    config.parity = QSerialPort::EvenParity;
    config.stopBits = QSerialPort::TwoStop;
    config.flowControl = QSerialPort::HardwareControl;
    config.dtrEnabled = true;
    config.rtsEnabled = false;

    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.connectSerialPort(config);

    QVERIFY(systemLogSpy.count() >= 1);
    const QString firstLog = systemLogSpy.first().at(0).toString();
    QVERIFY(firstLog.contains(QStringLiteral("正在打开串口")));
    QVERIFY(firstLog.contains(QStringLiteral("COM_DOES_NOT_EXIST 57600 7E2 硬件流控")));
    QVERIFY(firstLog.contains(QStringLiteral("DTR=on RTS=off")));
    QCOMPARE(controller.serialManager().session().config().portName,
             QStringLiteral("COM_DOES_NOT_EXIST"));
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
    QCOMPARE(errorSpy.takeFirst().at(0).toString(), QStringLiteral("串口端口名为空"));
    QCOMPARE(controller.serialManager().session().errorString(), QStringLiteral("串口端口名为空"));
}

void SerialStationControllerTest::invalidConfigConnectReportsConfigValidationReason_data()
{
    QTest::addColumn<SerialPortConfig>("config");
    QTest::addColumn<QString>("message");

    SerialPortConfig emptyPort;
    emptyPort.portName = QStringLiteral("  ");
    emptyPort.baudRate = 115200;
    QTest::newRow("empty-port") << emptyPort << QStringLiteral("串口端口名为空");

    SerialPortConfig badBaud;
    badBaud.portName = QStringLiteral("COM_BAD_BAUD");
    badBaud.baudRate = 0;
    QTest::newRow("bad-baud") << badBaud << QStringLiteral("串口波特率必须大于 0");

    SerialPortConfig badDataBits;
    badDataBits.portName = QStringLiteral("COM_BAD_DATA");
    badDataBits.dataBits = static_cast<QSerialPort::DataBits>(-1);
    QTest::newRow("bad-data-bits") << badDataBits << QStringLiteral("串口数据位不受支持");

    SerialPortConfig badParity;
    badParity.portName = QStringLiteral("COM_BAD_PARITY");
    badParity.parity = static_cast<QSerialPort::Parity>(-1);
    QTest::newRow("bad-parity") << badParity << QStringLiteral("串口校验位不受支持");

    SerialPortConfig badStopBits;
    badStopBits.portName = QStringLiteral("COM_BAD_STOP");
    badStopBits.stopBits = static_cast<QSerialPort::StopBits>(-1);
    QTest::newRow("bad-stop-bits") << badStopBits << QStringLiteral("串口停止位不受支持");

    SerialPortConfig badFlow;
    badFlow.portName = QStringLiteral("COM_BAD_FLOW");
    badFlow.flowControl = static_cast<QSerialPort::FlowControl>(-1);
    QTest::newRow("bad-flow") << badFlow << QStringLiteral("串口流控不受支持");
}

void SerialStationControllerTest::invalidConfigConnectReportsConfigValidationReason()
{
    QFETCH(SerialPortConfig, config);
    QFETCH(QString, message);

    SerialStationController controller;
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorOccurred);
    QSignalSpy stateSpy(&controller, &SerialStationController::serialStateChanged);
    QSignalSpy systemLogSpy(&controller, &SerialStationController::serialSystemLogged);
    QSignalSpy errorCountSpy(&controller, &SerialStationController::serialErrorCounted);

    controller.connectSerialPort(config);

    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(stateSpy.count(), 1);
    QCOMPARE(systemLogSpy.count(), 1);
    QCOMPARE(errorCountSpy.count(), 1);
    QCOMPARE(errorSpy.takeFirst().at(0).toString(), message);
    QCOMPARE(systemLogSpy.takeFirst().at(0).toString(), message);
    QCOMPARE(controller.serialManager().session().state(), SerialSessionState::Error);
    QCOMPARE(controller.serialManager().session().errorString(), message);
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
