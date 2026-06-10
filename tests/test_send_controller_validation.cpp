#include <QtTest/QtTest>

#include "core/send/SendController.h"
#include "serial/commands/SendHistory.h"
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
    void nonEmptySendReportsDisconnected();
};

namespace {
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

QTEST_MAIN(SendControllerValidationTest)
#include "test_send_controller_validation.moc"
