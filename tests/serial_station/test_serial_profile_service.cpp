#include <QtTest/QtTest>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>

#include "apps/serial_station/services/SerialProfileService.h"

using serial_station::SerialPortConfig;
using serial_station::SerialProfileCommand;
using serial_station::SerialProfileResult;
using serial_station::SerialProfileService;
using serial_station::SerialProfileWriteResult;
using serial_station::SerialStationProfile;

class SerialProfileServiceTest : public QObject {
    Q_OBJECT

private slots:
    void jsonRoundTripPreservesWorkbenchConfiguration();
    void commandsRemainOrderedAfterRoundTrip();
    void saveAndLoadProfileFile();
    void missingNameFailsValidation();
    void invalidPortConfigFailsValidation();
    void malformedJsonFailsParsing();
    void missingFileFailsLoading();
    void unsupportedProtocolNameFailsValidation();
    void unsupportedSendModeFailsValidation();
    void blankCommandNameFailsValidation();
};

static SerialStationProfile sampleProfile()
{
    SerialStationProfile profile;
    profile.schemaVersion = 1;
    profile.name = QStringLiteral("STM32 Bootloader");
    profile.description = QStringLiteral("UART 115200 8N1 bootloader session");
    profile.tags = {QStringLiteral("stm32"), QStringLiteral("bootloader")};
    profile.port.portName = QStringLiteral(" COM7 ");
    profile.port.baudRate = 115200;
    profile.port.dataBits = QSerialPort::Data8;
    profile.port.parity = QSerialPort::NoParity;
    profile.port.stopBits = QSerialPort::OneStop;
    profile.port.flowControl = QSerialPort::NoFlowControl;
    profile.port.dtrEnabled = true;
    profile.port.rtsEnabled = false;
    profile.protocolName = QStringLiteral("modbus_rtu");
    profile.sendMode = QStringLiteral("protocol");
    profile.commands = {
        {QStringLiteral("Read Version"), QStringLiteral("read_version"), QStringLiteral("ascii")},
        {QStringLiteral("Read Holding"), QStringLiteral("01 03 00 00 00 02"), QStringLiteral("hex")},
    };
    return profile;
}

void SerialProfileServiceTest::jsonRoundTripPreservesWorkbenchConfiguration()
{
    const SerialProfileService service;

    QString errorMessage;
    const QString json = service.toJson(sampleProfile(), &errorMessage);
    QVERIFY2(!json.isEmpty(), qPrintable(errorMessage));

    const SerialProfileResult result = service.fromJson(json);
    QVERIFY2(result.ok, qPrintable(result.errorMessage));
    QCOMPARE(result.profile.schemaVersion, 1);
    QCOMPARE(result.profile.name, QStringLiteral("STM32 Bootloader"));
    QCOMPARE(result.profile.description, QStringLiteral("UART 115200 8N1 bootloader session"));
    QCOMPARE(result.profile.tags, QStringList({QStringLiteral("stm32"), QStringLiteral("bootloader")}));
    QCOMPARE(result.profile.port.portName, QStringLiteral("COM7"));
    QCOMPARE(result.profile.port.baudRate, 115200);
    QCOMPARE(result.profile.port.dataBits, QSerialPort::Data8);
    QCOMPARE(result.profile.port.parity, QSerialPort::NoParity);
    QCOMPARE(result.profile.port.stopBits, QSerialPort::OneStop);
    QCOMPARE(result.profile.port.flowControl, QSerialPort::NoFlowControl);
    QVERIFY(result.profile.port.dtrEnabled);
    QVERIFY(!result.profile.port.rtsEnabled);
    QCOMPARE(result.profile.protocolName, QStringLiteral("modbus_rtu"));
    QCOMPARE(result.profile.sendMode, QStringLiteral("protocol"));
}

void SerialProfileServiceTest::commandsRemainOrderedAfterRoundTrip()
{
    const SerialProfileService service;

    QString errorMessage;
    const QString json = service.toJson(sampleProfile(), &errorMessage);
    QVERIFY2(!json.isEmpty(), qPrintable(errorMessage));
    const SerialProfileResult result = service.fromJson(json);

    QVERIFY2(result.ok, qPrintable(result.errorMessage));
    QCOMPARE(result.profile.commands.size(), 2);
    QCOMPARE(result.profile.commands.at(0).name, QStringLiteral("Read Version"));
    QCOMPARE(result.profile.commands.at(0).payload, QStringLiteral("read_version"));
    QCOMPARE(result.profile.commands.at(0).mode, QStringLiteral("ascii"));
    QCOMPARE(result.profile.commands.at(1).name, QStringLiteral("Read Holding"));
    QCOMPARE(result.profile.commands.at(1).payload, QStringLiteral("01 03 00 00 00 02"));
    QCOMPARE(result.profile.commands.at(1).mode, QStringLiteral("hex"));
}

void SerialProfileServiceTest::saveAndLoadProfileFile()
{
    const SerialProfileService service;
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    const QString path = QDir(tempDir.path()).filePath(QStringLiteral("bootloader.edserialprofile"));

    const SerialProfileWriteResult writeResult = service.saveToFile(sampleProfile(), path);
    QVERIFY2(writeResult.ok, qPrintable(writeResult.errorMessage));
    QCOMPARE(writeResult.filePath, path);
    QVERIFY(writeResult.bytesWritten > 0);
    QVERIFY(QFile::exists(path));

    const SerialProfileResult loadResult = service.loadFromFile(path);
    QVERIFY2(loadResult.ok, qPrintable(loadResult.errorMessage));
    QCOMPARE(loadResult.profile.name, QStringLiteral("STM32 Bootloader"));
    QCOMPARE(loadResult.profile.protocolName, QStringLiteral("modbus_rtu"));
}

void SerialProfileServiceTest::missingNameFailsValidation()
{
    SerialStationProfile profile = sampleProfile();
    profile.name = QStringLiteral("  ");

    const SerialProfileService service;
    QString errorMessage;
    const QString json = service.toJson(profile, &errorMessage);

    QVERIFY(json.isEmpty());
    QVERIFY(errorMessage.contains(QStringLiteral("名称")));
}

void SerialProfileServiceTest::invalidPortConfigFailsValidation()
{
    SerialStationProfile profile = sampleProfile();
    profile.port.portName.clear();

    const SerialProfileService service;
    QString errorMessage;
    const QString json = service.toJson(profile, &errorMessage);

    QVERIFY(json.isEmpty());
    QVERIFY(errorMessage.contains(QStringLiteral("串口")));
}

void SerialProfileServiceTest::malformedJsonFailsParsing()
{
    const SerialProfileService service;
    const SerialProfileResult result = service.fromJson(QStringLiteral("{\"name\":"));

    QVERIFY(!result.ok);
    QVERIFY(result.errorMessage.contains(QStringLiteral("JSON")));
}

void SerialProfileServiceTest::missingFileFailsLoading()
{
    const SerialProfileService service;
    const SerialProfileResult result = service.loadFromFile(QStringLiteral("Z:/missing/profile.json"));

    QVERIFY(!result.ok);
    QVERIFY(result.errorMessage.contains(QStringLiteral("读取")));
}

void SerialProfileServiceTest::unsupportedProtocolNameFailsValidation()
{
    SerialStationProfile profile = sampleProfile();
    profile.protocolName = QStringLiteral("unknown_protocol");

    const SerialProfileService service;
    QString errorMessage;
    const QString json = service.toJson(profile, &errorMessage);

    QVERIFY(json.isEmpty());
    QVERIFY(errorMessage.contains(QStringLiteral("协议")));
}

void SerialProfileServiceTest::unsupportedSendModeFailsValidation()
{
    SerialStationProfile profile = sampleProfile();
    profile.sendMode = QStringLiteral("binary");

    const SerialProfileService service;
    QString errorMessage;
    const QString json = service.toJson(profile, &errorMessage);

    QVERIFY(json.isEmpty());
    QVERIFY(errorMessage.contains(QStringLiteral("发送模式")));
}

void SerialProfileServiceTest::blankCommandNameFailsValidation()
{
    SerialStationProfile profile = sampleProfile();
    profile.commands.append({QStringLiteral("  "), QStringLiteral("PING"), QStringLiteral("ascii")});

    const SerialProfileService service;
    QString errorMessage;
    const QString json = service.toJson(profile, &errorMessage);

    QVERIFY(json.isEmpty());
    QVERIFY(errorMessage.contains(QStringLiteral("命令")));
}

QTEST_MAIN(SerialProfileServiceTest)
#include "test_serial_profile_service.moc"
