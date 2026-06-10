#include <QtTest/QtTest>

#include "core/send/SendController.h"
#include "connection/interface/IConnection.h"
#include "serial/commands/SendHistory.h"
#include "serial/commands/TimedSender.h"
#include "terminal/model/TerminalModel.h"
#include "utils/log/DataLogger.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>

class SendControllerValidationTest : public QObject {
    Q_OBJECT

private slots:
    void invalidHexMarksInputImmediately();
    void validHexClearsInputErrorImmediately();
    void textModeClearsHexInputError();
    void hexModeDisablesNewlineSelection();
    void textModeEnablesNewlineSelection();
    void hexModeShowsHexPlaceholder();
    void textModeRestoresTextPlaceholder();
    void emptySendReportsEmptyInputBeforeConnectionState();
    void invalidHexSendReportsFormatBeforeConnectionState();
    void nonEmptySendReportsDisconnected();
    void timedSendDisconnectedDoesNotCountAsSuccessfulTimedSend();
    void timedSendConnectedCountsAfterSuccessfulWrite();
    void quickCommandDisconnectedDoesNotCountAsMacroExecution();
    void quickCommandConnectedCountsAfterSuccessfulWrite();
};

namespace {
class FakeConnection : public IConnection {
    Q_OBJECT

public:
    explicit FakeConnection(ConnectionState state, QObject* parent = nullptr)
        : IConnection(parent)
        , m_state(state)
    {
    }

    ConnectionType type() const override { return ConnectionType::Serial; }
    QString name() const override { return QStringLiteral("FakeConnection"); }
    ConnectionState state() const override { return m_state; }
    bool open() override { m_state = ConnectionState::Connected; return true; }
    void close() override { m_state = ConnectionState::Disconnected; }
    qint64 write(const QByteArray& data) override
    {
        writtenData.append(data);
        return data.size();
    }
    void configure(const QVariantMap& params) override { Q_UNUSED(params); }

    QByteArray writtenData;

private:
    ConnectionState m_state;
};

struct SendBarFixture {
    QWidget parent;
    TerminalModel model;
    DataLogger logger;
    SendHistory history;
    SendController controller{&model, &logger, &history};
    QWidget* sendBar = nullptr;
    QLineEdit* input = nullptr;
    QComboBox* modeCombo = nullptr;
    QComboBox* newlineCombo = nullptr;

    SendBarFixture()
    {
        sendBar = controller.createSendBar(&parent);
        input = sendBar->findChild<QLineEdit*>("sendInput");
        modeCombo = sendBar->findChild<QComboBox*>("sendModeCombo");
        newlineCombo = sendBar->findChild<QComboBox*>("newlineCombo");
    }
};
} // namespace

void SendControllerValidationTest::invalidHexMarksInputImmediately()
{
    SendBarFixture fixture;
    QVERIFY(fixture.input);
    QVERIFY(fixture.modeCombo);

    fixture.modeCombo->setCurrentIndex(1);
    fixture.input->setText(QStringLiteral("AA 5"));

    QCOMPARE(fixture.input->property("hasError").toBool(), true);
}

void SendControllerValidationTest::validHexClearsInputErrorImmediately()
{
    SendBarFixture fixture;
    QVERIFY(fixture.input);
    QVERIFY(fixture.modeCombo);

    fixture.modeCombo->setCurrentIndex(1);
    fixture.input->setText(QStringLiteral("AA 5"));
    fixture.input->setText(QStringLiteral("AA 55"));

    QCOMPARE(fixture.input->property("hasError").toBool(), false);
}

void SendControllerValidationTest::textModeClearsHexInputError()
{
    SendBarFixture fixture;
    QVERIFY(fixture.input);
    QVERIFY(fixture.modeCombo);

    fixture.modeCombo->setCurrentIndex(1);
    fixture.input->setText(QStringLiteral("AA 5"));
    fixture.modeCombo->setCurrentIndex(0);

    QCOMPARE(fixture.input->property("hasError").toBool(), false);
}

void SendControllerValidationTest::hexModeDisablesNewlineSelection()
{
    SendBarFixture fixture;
    QVERIFY(fixture.modeCombo);
    QVERIFY(fixture.newlineCombo);

    fixture.modeCombo->setCurrentIndex(1);

    QVERIFY(!fixture.newlineCombo->isEnabled());
}

void SendControllerValidationTest::textModeEnablesNewlineSelection()
{
    SendBarFixture fixture;
    QVERIFY(fixture.modeCombo);
    QVERIFY(fixture.newlineCombo);

    fixture.modeCombo->setCurrentIndex(1);
    fixture.modeCombo->setCurrentIndex(0);

    QVERIFY(fixture.newlineCombo->isEnabled());
}

void SendControllerValidationTest::hexModeShowsHexPlaceholder()
{
    SendBarFixture fixture;
    QVERIFY(fixture.input);
    QVERIFY(fixture.modeCombo);

    fixture.modeCombo->setCurrentIndex(1);

    QVERIFY(fixture.input->placeholderText().contains(QStringLiteral("AA 55")));
}

void SendControllerValidationTest::textModeRestoresTextPlaceholder()
{
    SendBarFixture fixture;
    QVERIFY(fixture.input);
    QVERIFY(fixture.modeCombo);

    fixture.modeCombo->setCurrentIndex(1);
    fixture.modeCombo->setCurrentIndex(0);

    QCOMPARE(fixture.input->placeholderText(), QStringLiteral("输入要发送的数据..."));
}

void SendControllerValidationTest::emptySendReportsEmptyInputBeforeConnectionState()
{
    SendBarFixture fixture;
    QVERIFY(fixture.sendBar);
    auto* sendButton = fixture.sendBar->findChild<QPushButton*>("sendButton");
    QVERIFY(sendButton);
    QSignalSpy spy(&fixture.controller, &SendController::statusMessage);

    QTest::mouseClick(sendButton, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), QStringLiteral("发送内容为空"));
}

void SendControllerValidationTest::invalidHexSendReportsFormatBeforeConnectionState()
{
    SendBarFixture fixture;
    QVERIFY(fixture.input);
    QVERIFY(fixture.modeCombo);
    QVERIFY(fixture.sendBar);
    auto* sendButton = fixture.sendBar->findChild<QPushButton*>("sendButton");
    QVERIFY(sendButton);
    QSignalSpy spy(&fixture.controller, &SendController::statusMessage);

    fixture.modeCombo->setCurrentIndex(1);
    fixture.input->setText(QStringLiteral("AA 5"));
    QTest::mouseClick(sendButton, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().first().toString().contains(QStringLiteral("HEX 格式错误")));
    QCOMPARE(fixture.input->property("hasError").toBool(), true);
}

void SendControllerValidationTest::nonEmptySendReportsDisconnected()
{
    SendBarFixture fixture;
    QVERIFY(fixture.input);
    QVERIFY(fixture.sendBar);
    auto* sendButton = fixture.sendBar->findChild<QPushButton*>("sendButton");
    QVERIFY(sendButton);
    QSignalSpy spy(&fixture.controller, &SendController::statusMessage);

    fixture.input->setText(QStringLiteral("AT"));
    QTest::mouseClick(sendButton, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), QStringLiteral("发送失败: 未连接"));
}

void SendControllerValidationTest::timedSendDisconnectedDoesNotCountAsSuccessfulTimedSend()
{
    SendBarFixture fixture;
    QVERIFY(fixture.controller.timedSender());
    QSignalSpy spy(&fixture.controller, &SendController::statusMessage);

    fixture.controller.timedSender()->setInterval(1);
    fixture.controller.timedSender()->setData(QByteArray("AT"));
    fixture.controller.timedSender()->start();
    QTRY_VERIFY_WITH_TIMEOUT(spy.count() >= 1, 80);
    fixture.controller.timedSender()->stop();

    QCOMPARE(fixture.controller.totalTimedSends(), 0ULL);
    QCOMPARE(fixture.controller.totalSends(), 0ULL);
}

void SendControllerValidationTest::timedSendConnectedCountsAfterSuccessfulWrite()
{
    SendBarFixture fixture;
    FakeConnection connection(ConnectionState::Connected);
    fixture.controller.setConnection(&connection);

    fixture.controller.timedSender()->setInterval(1);
    fixture.controller.timedSender()->setData(QByteArray("AT"));
    fixture.controller.timedSender()->start();
    QTRY_VERIFY_WITH_TIMEOUT(fixture.controller.totalTimedSends() >= 1ULL, 80);
    fixture.controller.timedSender()->stop();

    QVERIFY(fixture.controller.totalTimedSends() >= 1ULL);
    QVERIFY(fixture.controller.totalSends() >= 1ULL);
    QVERIFY(connection.writtenData.contains(QByteArray("AT")));
}

void SendControllerValidationTest::quickCommandDisconnectedDoesNotCountAsMacroExecution()
{
    SendBarFixture fixture;
    QSignalSpy spy(&fixture.controller, &SendController::statusMessage);

    fixture.controller.onQuickCommand(QByteArray("AT"));

    QCOMPARE(spy.count(), 1);
    QCOMPARE(fixture.controller.totalMacroExecutions(), 0ULL);
    QCOMPARE(fixture.controller.totalSends(), 0ULL);
}

void SendControllerValidationTest::quickCommandConnectedCountsAfterSuccessfulWrite()
{
    SendBarFixture fixture;
    FakeConnection connection(ConnectionState::Connected);
    fixture.controller.setConnection(&connection);

    fixture.controller.onQuickCommand(QByteArray("AT"));

    QCOMPARE(fixture.controller.totalMacroExecutions(), 1ULL);
    QCOMPARE(fixture.controller.totalSends(), 1ULL);
    QCOMPARE(connection.writtenData, QByteArray("AT"));
}

QTEST_MAIN(SendControllerValidationTest)
#include "test_send_controller_validation.moc"
