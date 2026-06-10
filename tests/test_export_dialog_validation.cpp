#include <QtTest/QtTest>

#include "utils/export/ExportDialog.h"

#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>

class ExportDialogValidationTest : public QObject {
    Q_OBJECT

private slots:
    void exportButtonDisabledUntilPathHasText();
    void whitespacePathDoesNotEmitExportRequest();
    void validPathEmitsTrimmedExportRequest();
};

void ExportDialogValidationTest::exportButtonDisabledUntilPathHasText()
{
    ExportDialog dialog;
    auto* pathEdit = dialog.findChild<QLineEdit*>(QStringLiteral("pathEdit"));
    auto* exportBtn = dialog.findChild<QPushButton*>(QStringLiteral("exportBtn"));

    QVERIFY(pathEdit);
    QVERIFY(exportBtn);
    QVERIFY(!exportBtn->isEnabled());

    pathEdit->setText(QStringLiteral("   "));
    QVERIFY(!exportBtn->isEnabled());

    pathEdit->setText(QStringLiteral("capture.csv"));
    QVERIFY(exportBtn->isEnabled());
}

void ExportDialogValidationTest::whitespacePathDoesNotEmitExportRequest()
{
    ExportDialog dialog;
    auto* pathEdit = dialog.findChild<QLineEdit*>(QStringLiteral("pathEdit"));
    auto* exportBtn = dialog.findChild<QPushButton*>(QStringLiteral("exportBtn"));
    QSignalSpy spy(&dialog, &ExportDialog::exportRequested);

    QVERIFY(pathEdit);
    QVERIFY(exportBtn);
    pathEdit->setText(QStringLiteral("   "));

    QTest::mouseClick(exportBtn, Qt::LeftButton);

    QCOMPARE(spy.count(), 0);
    QCOMPARE(dialog.totalExports(), 0ULL);
}

void ExportDialogValidationTest::validPathEmitsTrimmedExportRequest()
{
    ExportDialog dialog;
    auto* pathEdit = dialog.findChild<QLineEdit*>(QStringLiteral("pathEdit"));
    auto* exportBtn = dialog.findChild<QPushButton*>(QStringLiteral("exportBtn"));
    QSignalSpy spy(&dialog, &ExportDialog::exportRequested);

    QVERIFY(pathEdit);
    QVERIFY(exportBtn);
    pathEdit->setText(QStringLiteral("  capture.csv  "));

    QTest::mouseClick(exportBtn, Qt::LeftButton);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().at(0).toString(), QStringLiteral("capture.csv"));
    QCOMPARE(dialog.totalExports(), 1ULL);
}

QTEST_MAIN(ExportDialogValidationTest)
#include "test_export_dialog_validation.moc"
