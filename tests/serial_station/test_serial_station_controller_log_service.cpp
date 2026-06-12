#include <QtTest/QtTest>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

#include "apps/serial_station/SerialStationConfig.h"
#include "apps/serial_station/SerialStationController.h"
#include "apps/serial_station/services/SerialLogService.h"

using serial_station::SerialLogDirection;
using serial_station::SerialLogFilter;
using serial_station::SerialLogRecord;
using serial_station::SerialPortConfig;
using serial_station::SerialStationController;

Q_DECLARE_METATYPE(SerialPortConfig)

class SerialStationControllerLogServiceTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void sendFailureIsStoredAsErrorJsonLine();
    void invalidHexFailureStoresCodecContext();
    void receiveLineIsStoredAsRxWithPayloadHex();
    void partialReceiveIsStoredAsSystemRecord();
    void invalidConnectConfigStoresValidationError();
    void clearRemovesRecordsAndAllowsNewAppend();
    void controllerFilterPreservesServiceBehavior();
    void logServiceAccessorCanLimitControllerRecords();
    void managerErrorIsRecordedWithoutExtraSystemSignal();
    void filteredJsonLinesOnlyContainsRequestedDirection();
    void logServiceMutableAccessorCanAppendDiagnosticRecord();
    void plainTextReflectsMixedControllerLogs();
    void jsonLinesExposeStableDirectionsForMixedLogs();
};

void SerialStationControllerLogServiceTest::initTestCase()
{
    qRegisterMetaType<SerialPortConfig>("SerialPortConfig");
}

void SerialStationControllerLogServiceTest::sendFailureIsStoredAsErrorJsonLine()
{
    SerialStationController controller;
    QSignalSpy failedSpy(&controller, &SerialStationController::serialCommandFailed);
    QSignalSpy systemSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.sendCommand(QStringLiteral(" AT+GMR "), QStringLiteral(" ASCII "));

    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(systemSpy.count(), 1);

    const QVector<SerialLogRecord> records = controller.logRecords();
    QCOMPARE(records.count(), 1);
    QCOMPARE(records.first().direction, SerialLogDirection::Error);
    QCOMPARE(records.first().source, QStringLiteral("controller"));
    QVERIFY(records.first().text.contains(QStringLiteral("AT+GMR")));
    QCOMPARE(records.first().fields.value(QStringLiteral("command")).toString(),
             QStringLiteral("AT+GMR"));
    QCOMPARE(records.first().fields.value(QStringLiteral("mode")).toString(),
             QStringLiteral("ascii"));

    const QJsonDocument document = QJsonDocument::fromJson(controller.logJsonLines().toUtf8());
    QVERIFY(document.isObject());
    QCOMPARE(document.object().value(QStringLiteral("direction")).toString(),
             QStringLiteral("error"));
    QVERIFY(document.object().value(QStringLiteral("text")).toString()
                .contains(QStringLiteral("串口未连接")));
}

void SerialStationControllerLogServiceTest::invalidHexFailureStoresCodecContext()
{
    SerialStationController controller;
    QSignalSpy failedSpy(&controller, &SerialStationController::serialCommandFailed);

    controller.sendCommand(QStringLiteral("01 ZZ"), QStringLiteral("hex"));

    QCOMPARE(failedSpy.count(), 1);

    const QVector<SerialLogRecord> records = controller.logRecords();
    QCOMPARE(records.count(), 1);
    QCOMPARE(records.first().direction, SerialLogDirection::Error);
    QCOMPARE(records.first().fields.value(QStringLiteral("command")).toString(),
             QStringLiteral("01 ZZ"));
    QCOMPARE(records.first().fields.value(QStringLiteral("mode")).toString(),
             QStringLiteral("hex"));
    QCOMPARE(records.first().fields.value(QStringLiteral("message")).toString(),
             QStringLiteral("HEX 内容包含非法字符"));
    QVERIFY(controller.logPlainText().contains(QStringLiteral("HEX 内容包含非法字符")));
}

void SerialStationControllerLogServiceTest::receiveLineIsStoredAsRxWithPayloadHex()
{
    SerialStationController controller;
    QSignalSpy rxSpy(&controller, &SerialStationController::serialRxLogged);
    QSignalSpy rxCountSpy(&controller, &SerialStationController::serialRxCounted);

    controller.handleBytesReceived(QByteArray("OK\n"));

    QCOMPARE(rxSpy.count(), 1);
    QCOMPARE(rxCountSpy.count(), 1);

    const QVector<SerialLogRecord> records = controller.logRecords();
    QCOMPARE(records.count(), 1);
    QCOMPARE(records.first().direction, SerialLogDirection::Rx);
    QCOMPARE(records.first().source, QStringLiteral("protocol"));
    QCOMPARE(records.first().text, QStringLiteral("OK"));
    QCOMPARE(records.first().payload, QByteArray("OK\n"));
    QCOMPARE(records.first().fields.value(QStringLiteral("eventType")).toString(),
             QStringLiteral("frame"));

    const QString json = controller.logJsonLines();
    QVERIFY(json.contains(QStringLiteral("\"direction\":\"rx\"")));
    QVERIFY(json.contains(QStringLiteral("\"payloadHex\":\"4F 4B 0A\"")));
}

void SerialStationControllerLogServiceTest::partialReceiveIsStoredAsSystemRecord()
{
    SerialStationController controller;
    QSignalSpy systemSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.handleBytesReceived(QByteArray("PART"));

    QCOMPARE(systemSpy.count(), 1);

    const QVector<SerialLogRecord> records = controller.logRecords();
    QCOMPARE(records.count(), 1);
    QCOMPARE(records.first().direction, SerialLogDirection::System);
    QVERIFY(records.first().text.contains(QStringLiteral("接收缓存")));
    QCOMPARE(records.first().fields.value(QStringLiteral("inputBytes")).toInt(), 4);
    QVERIFY(controller.logPlainText().contains(QStringLiteral("SYSTEM")));
}

void SerialStationControllerLogServiceTest::invalidConnectConfigStoresValidationError()
{
    SerialStationController controller;
    SerialPortConfig config;
    config.portName.clear();
    config.baudRate = 115200;
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorOccurred);
    QSignalSpy systemSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.connectSerialPort(config);

    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(systemSpy.count(), 1);

    const QVector<SerialLogRecord> records = controller.logRecords();
    QCOMPARE(records.count(), 2);
    QCOMPARE(records.at(0).direction, SerialLogDirection::Error);
    QCOMPARE(records.at(0).text, QStringLiteral("串口端口名为空"));
    QCOMPARE(records.at(1).direction, SerialLogDirection::System);
    QCOMPARE(records.at(1).text, QStringLiteral("串口端口名为空"));
}

void SerialStationControllerLogServiceTest::clearRemovesRecordsAndAllowsNewAppend()
{
    SerialStationController controller;

    controller.sendCommand(QStringLiteral("PING"), QStringLiteral("ascii"));
    controller.handleBytesReceived(QByteArray("OK\n"));
    QCOMPARE(controller.logRecords().count(), 2);

    controller.clearLogRecords();

    QVERIFY(controller.logRecords().isEmpty());
    QVERIFY(controller.logPlainText().isEmpty());
    QVERIFY(controller.logJsonLines().isEmpty());

    controller.handleBytesReceived(QByteArray("READY\n"));

    QCOMPARE(controller.logRecords().count(), 1);
    QCOMPARE(controller.logRecords().first().direction, SerialLogDirection::Rx);
    QCOMPARE(controller.logRecords().first().text, QStringLiteral("READY"));
}

void SerialStationControllerLogServiceTest::controllerFilterPreservesServiceBehavior()
{
    SerialStationController controller;

    controller.sendCommand(QStringLiteral("PING"), QStringLiteral("ascii"));
    controller.handleBytesReceived(QByteArray("OK\n"));
    controller.handleBytesReceived(QByteArray("PART"));

    SerialLogFilter rxFilter;
    rxFilter.directions = {SerialLogDirection::Rx};
    QCOMPARE(controller.logRecords(rxFilter).count(), 1);
    QCOMPARE(controller.logRecords(rxFilter).first().text, QStringLiteral("OK"));
    QVERIFY(controller.logPlainText(rxFilter).contains(QStringLiteral("OK")));
    QVERIFY(!controller.logPlainText(rxFilter).contains(QStringLiteral("PING")));

    SerialLogFilter keywordFilter;
    keywordFilter.textContains = QStringLiteral("PING");
    QCOMPARE(controller.logRecords(keywordFilter).count(), 1);
    QCOMPARE(controller.logRecords(keywordFilter).first().direction, SerialLogDirection::Error);
}

void SerialStationControllerLogServiceTest::logServiceAccessorCanLimitControllerRecords()
{
    SerialStationController controller;

    controller.logService().setMaxRecords(2);
    controller.sendCommand(QStringLiteral("FIRST"), QStringLiteral("ascii"));
    controller.handleBytesReceived(QByteArray("OK\n"));
    controller.handleBytesReceived(QByteArray("PART"));

    const QVector<SerialLogRecord> records = controller.logRecords();
    QCOMPARE(records.count(), 2);
    QCOMPARE(records.at(0).direction, SerialLogDirection::Rx);
    QCOMPARE(records.at(0).text, QStringLiteral("OK"));
    QCOMPARE(records.at(1).direction, SerialLogDirection::System);
    QVERIFY(records.at(1).text.contains(QStringLiteral("接收缓存")));
    QVERIFY(!controller.logPlainText().contains(QStringLiteral("FIRST")));
}

void SerialStationControllerLogServiceTest::managerErrorIsRecordedWithoutExtraSystemSignal()
{
    SerialStationController controller;
    SerialPortConfig config;
    config.portName = QStringLiteral("COM_FAKE");
    config.baudRate = 115200;
    QSignalSpy errorSpy(&controller, &SerialStationController::serialErrorOccurred);
    QSignalSpy systemSpy(&controller, &SerialStationController::serialSystemLogged);

    controller.serialManager().rejectConfiguration(config, QStringLiteral("模拟串口错误"));

    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(systemSpy.count(), 0);
    QCOMPARE(errorSpy.takeFirst().at(0).toString(), QStringLiteral("模拟串口错误"));

    const QVector<SerialLogRecord> records = controller.logRecords();
    QCOMPARE(records.count(), 1);
    QCOMPARE(records.first().direction, SerialLogDirection::Error);
    QCOMPARE(records.first().source, QStringLiteral("serial_manager"));
    QCOMPARE(records.first().text, QStringLiteral("模拟串口错误"));
}

void SerialStationControllerLogServiceTest::filteredJsonLinesOnlyContainsRequestedDirection()
{
    SerialStationController controller;

    controller.sendCommand(QStringLiteral("PING"), QStringLiteral("ascii"));
    controller.handleBytesReceived(QByteArray("OK\n"));
    controller.handleBytesReceived(QByteArray("PART"));

    SerialLogFilter rxFilter;
    rxFilter.directions = {SerialLogDirection::Rx};

    const QString jsonLines = controller.logJsonLines(rxFilter);
    const QStringList lines = jsonLines.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QCOMPARE(lines.count(), 1);

    const QJsonDocument document = QJsonDocument::fromJson(lines.first().toUtf8());
    QVERIFY(document.isObject());
    QCOMPARE(document.object().value(QStringLiteral("direction")).toString(),
             QStringLiteral("rx"));
    QCOMPARE(document.object().value(QStringLiteral("payloadHex")).toString(),
             QStringLiteral("4F 4B 0A"));
    QVERIFY(!jsonLines.contains(QStringLiteral("PING")));
    QVERIFY(!jsonLines.contains(QStringLiteral("接收缓存")));
}

void SerialStationControllerLogServiceTest::logServiceMutableAccessorCanAppendDiagnosticRecord()
{
    SerialStationController controller;

    QVariantMap fields;
    fields.insert(QStringLiteral("phase"), QStringLiteral("manual_diagnostic"));

    QVERIFY(controller.logService().appendSystem(QStringLiteral("人工诊断记录"),
                                                 QStringLiteral("test"),
                                                 fields));

    QCOMPARE(controller.logRecords().count(), 1);
    QCOMPARE(controller.logRecords().first().direction, SerialLogDirection::System);
    QCOMPARE(controller.logRecords().first().source, QStringLiteral("test"));
    QCOMPARE(controller.logRecords().first().fields.value(QStringLiteral("phase")).toString(),
             QStringLiteral("manual_diagnostic"));
    QVERIFY(controller.logPlainText().contains(QStringLiteral("人工诊断记录")));
    QVERIFY(controller.logJsonLines().contains(QStringLiteral("manual_diagnostic")));
}

void SerialStationControllerLogServiceTest::plainTextReflectsMixedControllerLogs()
{
    SerialStationController controller;

    controller.sendCommand(QStringLiteral("PING"), QStringLiteral("ascii"));
    controller.handleBytesReceived(QByteArray("OK\n"));
    controller.handleBytesReceived(QByteArray("PART"));

    const QString plainText = controller.logPlainText();
    QVERIFY(plainText.contains(QStringLiteral("ERROR")));
    QVERIFY(plainText.contains(QStringLiteral("RX")));
    QVERIFY(plainText.contains(QStringLiteral("SYSTEM")));
    QVERIFY(plainText.contains(QStringLiteral("PING")));
    QVERIFY(plainText.contains(QStringLiteral("OK")));
    QVERIFY(plainText.contains(QStringLiteral("hex=4F 4B 0A")));
    QVERIFY(plainText.contains(QStringLiteral("接收缓存")));
}

void SerialStationControllerLogServiceTest::jsonLinesExposeStableDirectionsForMixedLogs()
{
    SerialStationController controller;

    controller.sendCommand(QStringLiteral("PING"), QStringLiteral("ascii"));
    controller.handleBytesReceived(QByteArray("OK\n"));
    controller.handleBytesReceived(QByteArray("PART"));

    const QStringList lines = controller.logJsonLines().split(QLatin1Char('\n'));
    QCOMPARE(lines.count(), 3);

    QStringList directions;
    for (const QString& line : lines) {
        const QJsonDocument document = QJsonDocument::fromJson(line.toUtf8());
        QVERIFY(document.isObject());
        directions.append(document.object().value(QStringLiteral("direction")).toString());
    }

    QCOMPARE(directions.at(0), QStringLiteral("error"));
    QCOMPARE(directions.at(1), QStringLiteral("rx"));
    QCOMPARE(directions.at(2), QStringLiteral("system"));
}

QTEST_MAIN(SerialStationControllerLogServiceTest)
#include "test_serial_station_controller_log_service.moc"
