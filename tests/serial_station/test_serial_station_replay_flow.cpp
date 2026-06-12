#include <QtTest/QtTest>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>

#include "apps/serial_station/SerialStationController.h"
#include "apps/serial_station/SerialStationWindow.h"
#include "apps/serial_station/services/SerialLogService.h"
#include "apps/serial_station/services/SerialReplayService.h"

using serial_station::SerialLogDirection;
using serial_station::SerialReplayPlan;
using serial_station::SerialStationController;
using serial_station::SerialStationWindow;

class SerialStationReplayFlowTest : public QObject {
    Q_OBJECT

private slots:
    void controllerReplayFailsClearlyWhenLogIsEmpty();
    void controllerReplaySucceedsAfterTxRxRecordsExist();
    void controllerPreviewLogsSummaryAndEventDetails();
    void windowExposesReplayButton();
    void clickingReplayButtonAppendsPreviewFeedback();
    void clearingLogsBeforeReplayCausesEmptyLogFailure();
    void replayPreviewDoesNotSendBytesWhenSerialIsClosed();
    void replayPreviewSkipsSystemRecordsByDefault();
    void replayPreviewCanIncludeSystemRecords();
    void replayPreviewKeepsStructuredResultInspectable();
    void replayPreviewCanIncludeErrorRecords();
    void replayPreviewTruncatesLongEventLists();
    void replayPreviewPreservesEventOrder();
    void replayButtonOnEmptyWindowShowsFailure();
    void clearButtonRemovesControllerRecordsBeforeReplay();
    void repeatedReplayUsesCurrentControllerSnapshot();
    void replayPreviewFailureAddsErrorRecordFields();
    void replayPreviewSummaryRecordsStructuredFields();
    void replayPreviewEventRecordsKeepIndexFields();
};

namespace {

void seedReplayRecords(SerialStationController& controller)
{
    QVERIFY(controller.logService().appendTx(QStringLiteral("PING"),
                                             QByteArray("PING\n"),
                                             QStringLiteral("test")));
    QVERIFY(controller.logService().appendRx(QStringLiteral("OK"),
                                             QByteArray("OK\n"),
                                             QStringLiteral("test")));
}

QPlainTextEdit* logView(SerialStationWindow& window)
{
    auto* view = window.findChild<QPlainTextEdit*>(QStringLiteral("serialLogView"));
    Q_ASSERT(view != nullptr);
    return view;
}

QPushButton* replayButton(SerialStationWindow& window)
{
    auto* button = window.findChild<QPushButton*>(QStringLiteral("serialLogReplayButton"));
    Q_ASSERT(button != nullptr);
    return button;
}

QPushButton* clearButton(SerialStationWindow& window)
{
    auto* button = window.findChild<QPushButton*>(QStringLiteral("serialLogClearButton"));
    Q_ASSERT(button != nullptr);
    return button;
}

SerialStationController* controllerFor(SerialStationWindow& window)
{
    auto* controller = window.findChild<SerialStationController*>();
    Q_ASSERT(controller != nullptr);
    return controller;
}

} // namespace

void SerialStationReplayFlowTest::controllerReplayFailsClearlyWhenLogIsEmpty()
{
    SerialStationController controller;
    QSignalSpy systemSpy(&controller, &SerialStationController::serialSystemLogged);

    const SerialReplayPlan plan = controller.previewReplayPlan();

    QVERIFY(!plan.ok);
    QVERIFY(plan.errorMessage.contains(QStringLiteral("没有可回放")));
    QCOMPARE(systemSpy.count(), 1);
    QVERIFY(systemSpy.takeFirst().at(0).toString().contains(QStringLiteral("回放预览失败")));
    QCOMPARE(controller.logRecords().count(), 1);
    QCOMPARE(controller.logRecords().first().direction, SerialLogDirection::Error);
}

void SerialStationReplayFlowTest::controllerReplaySucceedsAfterTxRxRecordsExist()
{
    SerialStationController controller;
    seedReplayRecords(controller);

    const SerialReplayPlan plan = controller.previewReplayPlan();

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.count(), 2);
    QCOMPARE(plan.events.at(0).direction, SerialLogDirection::Tx);
    QCOMPARE(plan.events.at(1).direction, SerialLogDirection::Rx);
    QVERIFY(controller.logPlainText().contains(QStringLiteral("回放预览")));
}

void SerialStationReplayFlowTest::controllerPreviewLogsSummaryAndEventDetails()
{
    SerialStationController controller;
    seedReplayRecords(controller);
    QSignalSpy systemSpy(&controller, &SerialStationController::serialSystemLogged);

    const SerialReplayPlan plan = controller.previewReplayPlan();

    QVERIFY(plan.ok);
    QVERIFY(systemSpy.count() >= 3);

    const QString plainText = controller.logPlainText();
    QVERIFY(plainText.contains(QStringLiteral("回放预览: 2 个事件")));
    QVERIFY(plainText.contains(QStringLiteral("回放事件 1/2")));
    QVERIFY(plainText.contains(QStringLiteral("PING")));
    QVERIFY(plainText.contains(QStringLiteral("hex=50 49 4E 47 0A")));
}

void SerialStationReplayFlowTest::windowExposesReplayButton()
{
    SerialStationWindow window;

    auto* button = replayButton(window);

    QCOMPARE(button->objectName(), QStringLiteral("serialLogReplayButton"));
    QCOMPARE(button->text(), QStringLiteral("回放"));
}

void SerialStationReplayFlowTest::clickingReplayButtonAppendsPreviewFeedback()
{
    SerialStationWindow window;
    auto* controller = controllerFor(window);
    seedReplayRecords(*controller);

    QTest::mouseClick(replayButton(window), Qt::LeftButton);

    const QString text = logView(window)->toPlainText();
    QVERIFY(text.contains(QStringLiteral("回放预览")));
    QVERIFY(text.contains(QStringLiteral("回放事件")));
    QVERIFY(text.contains(QStringLiteral("PING")));
}

void SerialStationReplayFlowTest::clearingLogsBeforeReplayCausesEmptyLogFailure()
{
    SerialStationWindow window;
    auto* controller = controllerFor(window);
    seedReplayRecords(*controller);

    QTest::mouseClick(clearButton(window), Qt::LeftButton);
    QTest::mouseClick(replayButton(window), Qt::LeftButton);

    const QString text = logView(window)->toPlainText();
    QVERIFY(text.contains(QStringLiteral("回放预览失败")));
    QVERIFY(text.contains(QStringLiteral("没有可回放")));
    QCOMPARE(controller->logRecords().count(), 1);
    QCOMPARE(controller->logRecords().first().direction, SerialLogDirection::Error);
}

void SerialStationReplayFlowTest::replayPreviewDoesNotSendBytesWhenSerialIsClosed()
{
    SerialStationController controller;
    seedReplayRecords(controller);
    QSignalSpy sentSpy(&controller, &SerialStationController::serialCommandSent);
    QSignalSpy failedSpy(&controller, &SerialStationController::serialCommandFailed);

    const SerialReplayPlan plan = controller.previewReplayPlan();

    QVERIFY(plan.ok);
    QCOMPARE(sentSpy.count(), 0);
    QCOMPARE(failedSpy.count(), 0);
    QVERIFY(!controller.serialManager().session().isOpen());
}

void SerialStationReplayFlowTest::replayPreviewSkipsSystemRecordsByDefault()
{
    SerialStationController controller;
    seedReplayRecords(controller);
    QVERIFY(controller.logService().appendSystem(QStringLiteral("缓存提示"),
                                                 QStringLiteral("test")));

    const SerialReplayPlan plan = controller.previewReplayPlan();

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.count(), 2);
    QCOMPARE(plan.skippedRecords, 1);
}

void SerialStationReplayFlowTest::replayPreviewCanIncludeSystemRecords()
{
    SerialStationController controller;
    seedReplayRecords(controller);
    QVERIFY(controller.logService().appendSystem(QStringLiteral("缓存提示"),
                                                 QStringLiteral("test")));

    serial_station::SerialReplayOptions options;
    options.includeSystem = true;
    const SerialReplayPlan plan = controller.previewReplayPlan(options);

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.count(), 3);
    QCOMPARE(plan.events.last().direction, SerialLogDirection::System);
}

void SerialStationReplayFlowTest::replayPreviewKeepsStructuredResultInspectable()
{
    SerialStationController controller;
    seedReplayRecords(controller);

    const SerialReplayPlan plan = controller.previewReplayPlan();

    QVERIFY(plan.ok);
    QVERIFY(plan.totalDurationMs >= 0);
    QVERIFY(plan.errorMessage.isEmpty());
    QCOMPARE(plan.events.first().text, QStringLiteral("PING"));
    QCOMPARE(plan.events.first().payload, QByteArray("PING\n"));
    QCOMPARE(controller.logRecords().last().direction, SerialLogDirection::System);
}

void SerialStationReplayFlowTest::replayPreviewCanIncludeErrorRecords()
{
    SerialStationController controller;
    seedReplayRecords(controller);
    QVERIFY(controller.logService().appendError(QStringLiteral("CRC错误"),
                                                QStringLiteral("test")));

    serial_station::SerialReplayOptions options;
    options.includeError = true;
    const SerialReplayPlan plan = controller.previewReplayPlan(options);

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.count(), 3);
    QCOMPARE(plan.events.last().direction, SerialLogDirection::Error);
    QCOMPARE(plan.events.last().text, QStringLiteral("CRC错误"));
}

void SerialStationReplayFlowTest::replayPreviewTruncatesLongEventLists()
{
    SerialStationController controller;
    for (int index = 0; index < 7; ++index) {
        QVERIFY(controller.logService().appendTx(QStringLiteral("CMD_%1").arg(index),
                                                 QByteArray("X"),
                                                 QStringLiteral("test")));
    }

    const SerialReplayPlan plan = controller.previewReplayPlan();

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.count(), 7);

    const QString plainText = controller.logPlainText();
    QVERIFY(plainText.contains(QStringLiteral("回放事件 5/7")));
    QVERIFY(!plainText.contains(QStringLiteral("回放事件 6/7")));
    QVERIFY(plainText.contains(QStringLiteral("回放预览已截断")));
    QVERIFY(plainText.contains(QStringLiteral("还剩 2 个事件")));
}

void SerialStationReplayFlowTest::replayPreviewPreservesEventOrder()
{
    SerialStationController controller;
    QVERIFY(controller.logService().appendTx(QStringLiteral("FIRST"),
                                             QByteArray("1"),
                                             QStringLiteral("test")));
    QVERIFY(controller.logService().appendRx(QStringLiteral("SECOND"),
                                             QByteArray("2"),
                                             QStringLiteral("test")));
    QVERIFY(controller.logService().appendTx(QStringLiteral("THIRD"),
                                             QByteArray("3"),
                                             QStringLiteral("test")));

    const SerialReplayPlan plan = controller.previewReplayPlan();

    QVERIFY(plan.ok);
    QCOMPARE(plan.events.count(), 3);
    QCOMPARE(plan.events.at(0).text, QStringLiteral("FIRST"));
    QCOMPARE(plan.events.at(1).text, QStringLiteral("SECOND"));
    QCOMPARE(plan.events.at(2).text, QStringLiteral("THIRD"));
}

void SerialStationReplayFlowTest::replayButtonOnEmptyWindowShowsFailure()
{
    SerialStationWindow window;

    QTest::mouseClick(replayButton(window), Qt::LeftButton);

    const QString text = logView(window)->toPlainText();
    QVERIFY(text.contains(QStringLiteral("回放预览失败")));
    QVERIFY(text.contains(QStringLiteral("没有可回放")));
}

void SerialStationReplayFlowTest::clearButtonRemovesControllerRecordsBeforeReplay()
{
    SerialStationWindow window;
    auto* controller = controllerFor(window);
    seedReplayRecords(*controller);
    QCOMPARE(controller->logRecords().count(), 2);

    QTest::mouseClick(clearButton(window), Qt::LeftButton);

    QVERIFY(controller->logRecords().isEmpty());
}

void SerialStationReplayFlowTest::repeatedReplayUsesCurrentControllerSnapshot()
{
    SerialStationController controller;
    QVERIFY(controller.logService().appendTx(QStringLiteral("FIRST"),
                                             QByteArray("1"),
                                             QStringLiteral("test")));

    const SerialReplayPlan first = controller.previewReplayPlan();
    QVERIFY(first.ok);
    QCOMPARE(first.events.count(), 1);

    controller.clearLogRecords();
    QVERIFY(controller.logService().appendRx(QStringLiteral("SECOND"),
                                             QByteArray("2"),
                                             QStringLiteral("test")));

    const SerialReplayPlan second = controller.previewReplayPlan();
    QVERIFY(second.ok);
    QCOMPARE(second.events.count(), 1);
    QCOMPARE(second.events.first().direction, SerialLogDirection::Rx);
    QCOMPARE(second.events.first().text, QStringLiteral("SECOND"));
    QVERIFY(!controller.logPlainText().contains(QStringLiteral("FIRST")));
}

void SerialStationReplayFlowTest::replayPreviewFailureAddsErrorRecordFields()
{
    SerialStationController controller;

    const SerialReplayPlan plan = controller.previewReplayPlan();

    QVERIFY(!plan.ok);

    const auto records = controller.logRecords();
    QCOMPARE(records.count(), 1);
    QCOMPARE(records.first().direction, SerialLogDirection::Error);
    QVERIFY(records.first().fields.contains(QStringLiteral("message")));
    QCOMPARE(records.first().fields.value(QStringLiteral("message")).toString(),
             plan.errorMessage);
}

void SerialStationReplayFlowTest::replayPreviewSummaryRecordsStructuredFields()
{
    SerialStationController controller;
    seedReplayRecords(controller);

    const SerialReplayPlan plan = controller.previewReplayPlan();

    QVERIFY(plan.ok);

    const auto records = controller.logRecords();
    QVERIFY(records.count() >= 3);

    const auto summaryRecord = records.at(2);
    QCOMPARE(summaryRecord.direction, SerialLogDirection::System);
    QCOMPARE(summaryRecord.fields.value(QStringLiteral("events")).toInt(),
             plan.events.count());
    QCOMPARE(summaryRecord.fields.value(QStringLiteral("totalDurationMs")).toLongLong(),
             plan.totalDurationMs);
    QCOMPARE(summaryRecord.fields.value(QStringLiteral("skippedRecords")).toInt(),
             plan.skippedRecords);
}

void SerialStationReplayFlowTest::replayPreviewEventRecordsKeepIndexFields()
{
    SerialStationController controller;
    seedReplayRecords(controller);

    const SerialReplayPlan plan = controller.previewReplayPlan();

    QVERIFY(plan.ok);

    const auto records = controller.logRecords();
    QVERIFY(records.count() >= 5);

    const auto firstEventRecord = records.at(3);
    const auto secondEventRecord = records.at(4);
    QCOMPARE(firstEventRecord.fields.value(QStringLiteral("eventIndex")).toInt(), 0);
    QCOMPARE(firstEventRecord.fields.value(QStringLiteral("eventCount")).toInt(), 2);
    QCOMPARE(secondEventRecord.fields.value(QStringLiteral("eventIndex")).toInt(), 1);
    QCOMPARE(secondEventRecord.fields.value(QStringLiteral("eventCount")).toInt(), 2);
    QVERIFY(firstEventRecord.text.contains(QStringLiteral("回放事件 1/2")));
    QVERIFY(secondEventRecord.text.contains(QStringLiteral("回放事件 2/2")));
}

QTEST_MAIN(SerialStationReplayFlowTest)
#include "test_serial_station_replay_flow.moc"
