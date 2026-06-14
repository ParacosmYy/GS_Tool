#include <QtTest/QtTest>

#include <memory>
#include <QtCore/QList>

#include "apps/serial_station/SerialStationConfig.h"
#include "apps/serial_station/SerialStationModels.h"
#include "apps/serial_station/core/SerialPort.h"
#include "apps/serial_station/core/SerialManager.h"

using serial_station::ISerialPort;
using serial_station::SerialManager;
using serial_station::SerialPortConfig;
using serial_station::SerialSessionState;

Q_DECLARE_METATYPE(SerialSessionState)

namespace {

class FakeSerialPort final : public ISerialPort {
    Q_OBJECT

public:
    explicit FakeSerialPort(QObject* parent = nullptr)
        : ISerialPort(parent)
    {
    }

    void configure(const SerialPortConfig& config) override
    {
        ++configureCalls;
        lastConfig = config;
    }

    SerialPortConfig config() const override
    {
        return lastConfig;
    }

    bool open() override
    {
        ++openCalls;
        if (!openAllowed) {
            m_errorString = openErrorMessage;
            return false;
        }

        m_isOpen = true;
        m_errorString.clear();
        return true;
    }

    void close() override
    {
        ++closeCalls;
        m_isOpen = false;
    }

    bool isOpen() const override
    {
        return m_isOpen;
    }

    qint64 write(const QByteArray& bytes) override
    {
        ++writeCalls;
        writtenPayloads.append(bytes);

        if (!m_isOpen || !writeAllowed) {
            m_errorString = writeErrorMessage;
            return -1;
        }

        if (!writeScriptResults.isEmpty()) {
            const qint64 scriptedResult = writeScriptResults.takeFirst();
            if (scriptedResult > 0) {
                return scriptedResult;
            }

            m_errorString = writeErrorMessage;
            return scriptedResult;
        }

        lastWritten = bytes;
        return bytes.size();
    }

    QString errorString() const override
    {
        return m_errorString;
    }

    void pushIncomingBytes(const QByteArray& bytes)
    {
        emit bytesReceived(bytes);
    }

    void pushError(const QString& message)
    {
        m_errorString = message;
        emit errorOccurred(message);
    }

    void failOpen(const QString& message)
    {
        openAllowed = false;
        openErrorMessage = message;
        m_errorString = message;
    }

    void failWrite(const QString& message)
    {
        writeAllowed = false;
        writeErrorMessage = message;
        m_errorString = message;
    }

    void enqueueWriteResults(const QList<qint64>& results)
    {
        writeScriptResults = results;
    }

    int configureCalls = 0;
    int openCalls = 0;
    int closeCalls = 0;
    int writeCalls = 0;
    SerialPortConfig lastConfig;
    QByteArray lastWritten;
    QList<QByteArray> writtenPayloads;
    bool openAllowed = true;
    bool writeAllowed = true;

private:
    bool m_isOpen = false;
    QString m_errorString;
    QString openErrorMessage = QStringLiteral("open failed");
    QString writeErrorMessage = QStringLiteral("write failed");
    QList<qint64> writeScriptResults;
};

SerialSessionState stateAt(const QSignalSpy& spy, int index)
{
    return spy.at(index).at(0).value<SerialSessionState>();
}

} // namespace

class SerialManagerTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void configureNormalizesAndForwardsToTransport();
    void openTransitionsSessionAndCallsTransport();
    void openFailurePropagatesTransportError();
    void sendForwardsBytesToTransport();
    void sendRetriesPartialWritesUntilSuccess();
    void sendReturnsErrorAfterWriteRetriesExhausted();
    void closeTransitionsBackToClosed();
    void incomingBytesPropagateThroughSignal();
    void transportErrorSignalMarksErrorState();
};

void SerialManagerTest::initTestCase()
{
    qRegisterMetaType<SerialSessionState>("SerialSessionState");
}

void SerialManagerTest::configureNormalizesAndForwardsToTransport()
{
    auto transport = std::make_unique<FakeSerialPort>();
    FakeSerialPort* rawTransport = transport.get();
    SerialManager manager(std::move(transport));

    SerialPortConfig config;
    config.portName = QStringLiteral("  COM9  ");
    config.baudRate = 57600;

    manager.configure(config);

    QCOMPARE(manager.session().config().portName, QStringLiteral("COM9"));
    QCOMPARE(rawTransport->configureCalls, 1);
    QCOMPARE(rawTransport->lastConfig.portName, QStringLiteral("COM9"));
    QCOMPARE(rawTransport->lastConfig.baudRate, 57600);
}

void SerialManagerTest::openTransitionsSessionAndCallsTransport()
{
    auto transport = std::make_unique<FakeSerialPort>();
    FakeSerialPort* rawTransport = transport.get();
    SerialManager manager(std::move(transport));
    QSignalSpy stateSpy(&manager, &SerialManager::stateChanged);

    QVERIFY(manager.open());

    QCOMPARE(rawTransport->openCalls, 1);
    QCOMPARE(stateSpy.count(), 2);
    QCOMPARE(stateAt(stateSpy, 0), SerialSessionState::Opening);
    QCOMPARE(stateAt(stateSpy, 1), SerialSessionState::Open);
    QCOMPARE(manager.session().state(), SerialSessionState::Open);
}

void SerialManagerTest::openFailurePropagatesTransportError()
{
    auto transport = std::make_unique<FakeSerialPort>();
    FakeSerialPort* rawTransport = transport.get();
    rawTransport->failOpen(QStringLiteral("cannot open"));
    SerialManager manager(std::move(transport));
    QSignalSpy stateSpy(&manager, &SerialManager::stateChanged);

    QVERIFY(!manager.open());

    QCOMPARE(rawTransport->openCalls, 1);
    QCOMPARE(stateSpy.count(), 2);
    QCOMPARE(stateAt(stateSpy, 0), SerialSessionState::Opening);
    QCOMPARE(stateAt(stateSpy, 1), SerialSessionState::Error);
    QCOMPARE(manager.session().state(), SerialSessionState::Error);
    QCOMPARE(manager.session().errorString(), QStringLiteral("cannot open"));
}

void SerialManagerTest::sendForwardsBytesToTransport()
{
    auto transport = std::make_unique<FakeSerialPort>();
    FakeSerialPort* rawTransport = transport.get();
    SerialManager manager(std::move(transport));
    QVERIFY(manager.open());

    const QByteArray payload("hello");
    QCOMPARE(manager.send(payload), payload.size());
    QCOMPARE(rawTransport->writeCalls, 1);
    QCOMPARE(rawTransport->lastWritten, payload);
}

void SerialManagerTest::sendRetriesPartialWritesUntilSuccess()
{
    auto transport = std::make_unique<FakeSerialPort>();
    FakeSerialPort* rawTransport = transport.get();
    SerialManager manager(std::move(transport));
    QVERIFY(manager.open());

    rawTransport->enqueueWriteResults({2, 0, 3});
    const QByteArray payload("hello");

    QCOMPARE(manager.send(payload), payload.size());
    QCOMPARE(rawTransport->writeCalls, 3);
    QCOMPARE(rawTransport->writtenPayloads.size(), 3);
    QCOMPARE(rawTransport->writtenPayloads.at(0), QByteArray("hello"));
    QCOMPARE(rawTransport->writtenPayloads.at(1), QByteArray("llo"));
    QCOMPARE(rawTransport->writtenPayloads.at(2), QByteArray("llo"));
}

void SerialManagerTest::sendReturnsErrorAfterWriteRetriesExhausted()
{
    auto transport = std::make_unique<FakeSerialPort>();
    FakeSerialPort* rawTransport = transport.get();
    SerialManager manager(std::move(transport));
    QVERIFY(manager.open());

    rawTransport->enqueueWriteResults({2, -1, -1, -1});
    const QByteArray payload("hello");

    QCOMPARE(manager.send(payload), 2);
    QCOMPARE(rawTransport->writeCalls, 4);
    QCOMPARE(rawTransport->writtenPayloads.at(0), QByteArray("hello"));
    QCOMPARE(rawTransport->writtenPayloads.at(1), QByteArray("llo"));
    QCOMPARE(rawTransport->writtenPayloads.at(2), QByteArray("llo"));
    QCOMPARE(rawTransport->writtenPayloads.at(3), QByteArray("llo"));
}

void SerialManagerTest::closeTransitionsBackToClosed()
{
    auto transport = std::make_unique<FakeSerialPort>();
    FakeSerialPort* rawTransport = transport.get();
    SerialManager manager(std::move(transport));
    QVERIFY(manager.open());
    QSignalSpy stateSpy(&manager, &SerialManager::stateChanged);

    manager.close();

    QCOMPARE(rawTransport->closeCalls, 1);
    QCOMPARE(manager.session().state(), SerialSessionState::Closed);
    QCOMPARE(stateSpy.count(), 1);
    QCOMPARE(stateAt(stateSpy, 0), SerialSessionState::Closed);
}

void SerialManagerTest::incomingBytesPropagateThroughSignal()
{
    auto transport = std::make_unique<FakeSerialPort>();
    FakeSerialPort* rawTransport = transport.get();
    SerialManager manager(std::move(transport));
    QSignalSpy bytesSpy(&manager, &SerialManager::bytesReceived);

    rawTransport->pushIncomingBytes(QByteArray("OK"));

    QCOMPARE(bytesSpy.count(), 1);
    QCOMPARE(bytesSpy.takeFirst().at(0).toByteArray(), QByteArray("OK"));
}

void SerialManagerTest::transportErrorSignalMarksErrorState()
{
    auto transport = std::make_unique<FakeSerialPort>();
    FakeSerialPort* rawTransport = transport.get();
    SerialManager manager(std::move(transport));
    QVERIFY(manager.open());
    QSignalSpy errorSpy(&manager, &SerialManager::errorOccurred);
    QSignalSpy stateSpy(&manager, &SerialManager::stateChanged);

    rawTransport->pushError(QStringLiteral("loopback disconnected"));

    QCOMPARE(errorSpy.count(), 1);
    QCOMPARE(errorSpy.takeFirst().at(0).toString(), QStringLiteral("loopback disconnected"));
    QCOMPARE(manager.session().state(), SerialSessionState::Error);
    QCOMPARE(stateSpy.count(), 1);
    QCOMPARE(stateAt(stateSpy, 0), SerialSessionState::Error);
}

QTEST_MAIN(SerialManagerTest)
#include "test_serial_manager.moc"
