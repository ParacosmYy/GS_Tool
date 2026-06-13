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
    void displaysTrendLines();
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

void SerialMeasurementPanelTest::displaysTrendLines()
{
    SerialMeasurementPanel panel;
    auto* trendLabel = panel.findChild<QLabel*>(QStringLiteral("serialMeasurementTrendLabel"));
    auto* trendView = panel.findChild<QPlainTextEdit*>(QStringLiteral("serialMeasurementTrendView"));

    QVERIFY(trendLabel != nullptr);
    QVERIFY(trendView != nullptr);

    panel.setTrendLines({QStringLiteral("#1 ch1=1 ch2=2"),
                         QStringLiteral("#2 ch1=3 ch2=4")});

    QVERIFY(trendLabel->text().contains(QStringLiteral("最近")));
    QVERIFY(trendView->toPlainText().contains(QStringLiteral("#1")));
    QVERIFY(trendView->toPlainText().contains(QStringLiteral("ch2=4")));
}

void SerialMeasurementPanelTest::clearRestoresEmptyState()
{
    SerialMeasurementPanel panel;
    auto* summary = panel.findChild<QLabel*>(QStringLiteral("serialMeasurementSummaryLabel"));
    auto* view = panel.findChild<QPlainTextEdit*>(QStringLiteral("serialMeasurementView"));
    auto* trendView = panel.findChild<QPlainTextEdit*>(QStringLiteral("serialMeasurementTrendView"));

    panel.setSummaryLines({QStringLiteral("ch1 latest=1")});
    panel.setTrendLines({QStringLiteral("#1 ch1=1")});
    panel.clear();

    QVERIFY(summary->text().contains(QStringLiteral("暂无测量数据")));
    QVERIFY(view->toPlainText().isEmpty());
    QVERIFY(trendView->toPlainText().isEmpty());
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
