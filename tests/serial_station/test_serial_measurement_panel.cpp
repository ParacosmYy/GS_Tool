#include <QtTest/QtTest>

#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>

#include "apps/serial_station/ui/SerialMeasurementPanel.h"

using namespace serial_station;

class SerialMeasurementPanelTest : public QObject {
    Q_OBJECT

private slots:
    void startsWithEmptyState();
    void displaysSummaryLines();
    void clearRestoresEmptyState();
    void emptySummaryRestoresEmptyState();
};

void SerialMeasurementPanelTest::startsWithEmptyState()
{
    SerialMeasurementPanel panel;
    auto* summary = panel.findChild<QLabel*>(QStringLiteral("serialMeasurementSummaryLabel"));
    auto* view = panel.findChild<QPlainTextEdit*>(QStringLiteral("serialMeasurementView"));

    QVERIFY(summary != nullptr);
    QVERIFY(view != nullptr);
    QVERIFY(summary->text().contains(QStringLiteral("暂无测量数据")));
    QVERIFY(view->toPlainText().isEmpty());
}

void SerialMeasurementPanelTest::displaysSummaryLines()
{
    SerialMeasurementPanel panel;
    auto* summary = panel.findChild<QLabel*>(QStringLiteral("serialMeasurementSummaryLabel"));
    auto* view = panel.findChild<QPlainTextEdit*>(QStringLiteral("serialMeasurementView"));

    panel.setSummaryLines({QStringLiteral("ch1 latest=1.25 samples=1 min=1.25 max=1.25"),
                           QStringLiteral("ch2 latest=-2.5 samples=1 min=-2.5 max=-2.5")});

    QVERIFY(summary->text().contains(QStringLiteral("2")));
    QVERIFY(view->toPlainText().contains(QStringLiteral("ch1")));
    QVERIFY(view->toPlainText().contains(QStringLiteral("ch2")));
}

void SerialMeasurementPanelTest::clearRestoresEmptyState()
{
    SerialMeasurementPanel panel;
    auto* summary = panel.findChild<QLabel*>(QStringLiteral("serialMeasurementSummaryLabel"));
    auto* view = panel.findChild<QPlainTextEdit*>(QStringLiteral("serialMeasurementView"));

    panel.setSummaryLines({QStringLiteral("ch1 latest=1")});
    panel.clear();

    QVERIFY(summary->text().contains(QStringLiteral("暂无测量数据")));
    QVERIFY(view->toPlainText().isEmpty());
}

void SerialMeasurementPanelTest::emptySummaryRestoresEmptyState()
{
    SerialMeasurementPanel panel;
    auto* summary = panel.findChild<QLabel*>(QStringLiteral("serialMeasurementSummaryLabel"));
    auto* view = panel.findChild<QPlainTextEdit*>(QStringLiteral("serialMeasurementView"));

    panel.setSummaryLines({QStringLiteral("ch1 latest=1")});
    panel.setSummaryLines(QStringList());

    QVERIFY(summary->text().contains(QStringLiteral("暂无测量数据")));
    QVERIFY(view->toPlainText().isEmpty());
}

QTEST_MAIN(SerialMeasurementPanelTest)
#include "test_serial_measurement_panel.moc"
