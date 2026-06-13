#include <QtTest/QtTest>

#include "apps/serial_station/services/SerialMeasurementService.h"

using namespace serial_station;

class SerialMeasurementServiceTest : public QObject {
    Q_OBJECT

private slots:
    void startsEmpty();
    void ignoresNonMeasurementEvents();
    void ignoresMeasurementWithoutValues();
    void recordsSingleChannelMeasurement();
    void recordsMultipleChannels();
    void updatesMinMaxAndSampleCount();
    void resetClearsSnapshot();
    void displayLinesContainChannelStats();
};

namespace {

SerialProtocolEvent measurementEvent(std::initializer_list<double> values)
{
    QVariantList variants;
    for (const double value : values) {
        variants.append(value);
    }

    SerialProtocolEvent event;
    event.type = QStringLiteral("measurement");
    event.protocolName = QStringLiteral("just_float");
    event.payload.insert(QStringLiteral("values"), variants);
    event.payload.insert(QStringLiteral("format"), QStringLiteral("just_float"));
    return event;
}

} // namespace

void SerialMeasurementServiceTest::startsEmpty()
{
    SerialMeasurementService service;

    QVERIFY(service.snapshot().isEmpty());
    QVERIFY(service.displayLines().isEmpty());
}

void SerialMeasurementServiceTest::ignoresNonMeasurementEvents()
{
    SerialMeasurementService service;
    SerialProtocolEvent event;
    event.type = QStringLiteral("frame");

    QVERIFY(!service.appendEvent(event));
    QVERIFY(service.snapshot().isEmpty());
}

void SerialMeasurementServiceTest::ignoresMeasurementWithoutValues()
{
    SerialMeasurementService service;
    SerialProtocolEvent event;
    event.type = QStringLiteral("measurement");

    QVERIFY(!service.appendEvent(event));
    QVERIFY(service.snapshot().isEmpty());
}

void SerialMeasurementServiceTest::recordsSingleChannelMeasurement()
{
    SerialMeasurementService service;

    QVERIFY(service.appendEvent(measurementEvent({1.5})));

    const SerialMeasurementSnapshot snapshot = service.snapshot();
    QCOMPARE(snapshot.protocolName, QStringLiteral("just_float"));
    QCOMPARE(snapshot.frameCount, 1);
    QCOMPARE(snapshot.channels.size(), 1);
    QCOMPARE(snapshot.channels.first().name, QStringLiteral("ch1"));
    QCOMPARE(snapshot.channels.first().latest, 1.5);
    QCOMPARE(snapshot.channels.first().minimum, 1.5);
    QCOMPARE(snapshot.channels.first().maximum, 1.5);
    QCOMPARE(snapshot.channels.first().sampleCount, 1);
}

void SerialMeasurementServiceTest::recordsMultipleChannels()
{
    SerialMeasurementService service;

    QVERIFY(service.appendEvent(measurementEvent({1.0, 2.0, 3.0})));

    const SerialMeasurementSnapshot snapshot = service.snapshot();
    QCOMPARE(snapshot.channels.size(), 3);
    QCOMPARE(snapshot.channels.at(0).name, QStringLiteral("ch1"));
    QCOMPARE(snapshot.channels.at(1).name, QStringLiteral("ch2"));
    QCOMPARE(snapshot.channels.at(2).name, QStringLiteral("ch3"));
    QCOMPARE(snapshot.channels.at(2).latest, 3.0);
}

void SerialMeasurementServiceTest::updatesMinMaxAndSampleCount()
{
    SerialMeasurementService service;

    QVERIFY(service.appendEvent(measurementEvent({4.0, -1.0})));
    QVERIFY(service.appendEvent(measurementEvent({2.0, 5.0})));
    QVERIFY(service.appendEvent(measurementEvent({8.0, 3.0})));

    const SerialMeasurementSnapshot snapshot = service.snapshot();
    QCOMPARE(snapshot.frameCount, 3);
    QCOMPARE(snapshot.channels.at(0).latest, 8.0);
    QCOMPARE(snapshot.channels.at(0).minimum, 2.0);
    QCOMPARE(snapshot.channels.at(0).maximum, 8.0);
    QCOMPARE(snapshot.channels.at(0).sampleCount, 3);
    QCOMPARE(snapshot.channels.at(1).minimum, -1.0);
    QCOMPARE(snapshot.channels.at(1).maximum, 5.0);
}

void SerialMeasurementServiceTest::resetClearsSnapshot()
{
    SerialMeasurementService service;
    service.appendEvent(measurementEvent({1.0}));

    service.reset();

    QVERIFY(service.snapshot().isEmpty());
}

void SerialMeasurementServiceTest::displayLinesContainChannelStats()
{
    SerialMeasurementService service;
    service.appendEvent(measurementEvent({1.25, -2.5}));

    const QStringList lines = service.displayLines();

    QCOMPARE(lines.size(), 2);
    QVERIFY(lines.at(0).contains(QStringLiteral("ch1")));
    QVERIFY(lines.at(0).contains(QStringLiteral("1.25")));
    QVERIFY(lines.at(0).contains(QStringLiteral("min")));
    QVERIFY(lines.at(1).contains(QStringLiteral("ch2")));
    QVERIFY(lines.at(1).contains(QStringLiteral("-2.5")));
}

QTEST_MAIN(SerialMeasurementServiceTest)
#include "test_serial_measurement_service.moc"
