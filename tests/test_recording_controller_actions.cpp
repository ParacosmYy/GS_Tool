#include <QtTest/QtTest>

#include "core/recording/RecordingController.h"

#include <QAction>
#include <QToolBar>

class RecordingControllerActionsTest : public QObject {
    Q_OBJECT

private slots:
    void recordActionFollowsConnectionState();
};

void RecordingControllerActionsTest::recordActionFollowsConnectionState()
{
    RecordingController controller(nullptr);
    QToolBar toolbar;
    controller.setupActions(&toolbar);

    const QList<QAction*> actions = toolbar.actions();
    QCOMPARE(actions.size(), 4);
    QAction* recordAction = actions.at(0);
    QAction* stopRecordAction = actions.at(1);
    QAction* playbackAction = actions.at(2);

    QVERIFY(!recordAction->isEnabled());
    QVERIFY(!stopRecordAction->isEnabled());
    QVERIFY(playbackAction->isEnabled());

    controller.setConnected(true);
    QVERIFY(recordAction->isEnabled());
    QVERIFY(!stopRecordAction->isEnabled());

    controller.setConnected(false);
    QVERIFY(!recordAction->isEnabled());
    QVERIFY(!recordAction->isChecked());
    QVERIFY(!stopRecordAction->isEnabled());
    QVERIFY(playbackAction->isEnabled());
}

QTEST_MAIN(RecordingControllerActionsTest)
#include "test_recording_controller_actions.moc"
