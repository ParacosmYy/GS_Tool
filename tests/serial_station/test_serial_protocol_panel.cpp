#include <QtTest/QtTest>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QLabel>

#include "apps/serial_station/ui/SerialProtocolPanel.h"

using serial_station::SerialProtocolPanel;

class SerialProtocolPanelTest : public QObject {
    Q_OBJECT

private slots:
    void startsWithEmptyStatus();
    void setProtocolsShowsActiveProtocol();
    void activatedProtocolEmitsSelection();
    void protocolNamesAreTrimmedBeforeDisplay();
    void setProtocolsDoesNotEmitSelection();
    void missingActiveProtocolKeepsCurrentSelection();
    void emptyProtocolListDisablesCombo();
    void invalidActivationDoesNotEmitSelection();
};

void SerialProtocolPanelTest::startsWithEmptyStatus()
{
    SerialProtocolPanel panel;

    auto* combo = panel.findChild<QComboBox*>(QStringLiteral("serialProtocolCombo"));
    auto* status = panel.findChild<QLabel*>(QStringLiteral("serialProtocolStatusLabel"));
    QVERIFY(combo != nullptr);
    QVERIFY(status != nullptr);
    QCOMPARE(combo->count(), 0);
    QCOMPARE(status->text(), QStringLiteral("未加载协议"));
}

void SerialProtocolPanelTest::setProtocolsShowsActiveProtocol()
{
    SerialProtocolPanel panel;
    panel.setProtocols({QStringLiteral("ascii_text"),
                        QStringLiteral("custom_md"),
                        QStringLiteral("modbus_rtu")},
                       QStringLiteral("custom_md"));

    auto* combo = panel.findChild<QComboBox*>(QStringLiteral("serialProtocolCombo"));
    auto* status = panel.findChild<QLabel*>(QStringLiteral("serialProtocolStatusLabel"));
    QVERIFY(combo != nullptr);
    QVERIFY(status != nullptr);
    QCOMPARE(combo->count(), 3);
    QCOMPARE(panel.activeProtocol(), QStringLiteral("custom_md"));
    QVERIFY(status->text().contains(QStringLiteral("custom_md")));
    QVERIFY(combo->isEnabled());
}

void SerialProtocolPanelTest::activatedProtocolEmitsSelection()
{
    SerialProtocolPanel panel;
    panel.setProtocols({QStringLiteral("ascii_text"),
                        QStringLiteral("custom_md"),
                        QStringLiteral("modbus_rtu")},
                       QStringLiteral("ascii_text"));
    auto* combo = panel.findChild<QComboBox*>(QStringLiteral("serialProtocolCombo"));
    QVERIFY(combo != nullptr);

    QSignalSpy selectedSpy(&panel, &SerialProtocolPanel::protocolSelected);
    const int customIndex = combo->findData(QStringLiteral("custom_md"));
    QVERIFY(customIndex >= 0);
    combo->setCurrentIndex(customIndex);
    emit combo->activated(customIndex);

    QCOMPARE(selectedSpy.count(), 1);
    QCOMPARE(selectedSpy.takeFirst().at(0).toString(), QStringLiteral("custom_md"));
    QCOMPARE(panel.activeProtocol(), QStringLiteral("custom_md"));
}

void SerialProtocolPanelTest::protocolNamesAreTrimmedBeforeDisplay()
{
    SerialProtocolPanel panel;
    panel.setProtocols({QStringLiteral("  ascii_text  "),
                        QStringLiteral(" "),
                        QStringLiteral(" custom_md")},
                       QStringLiteral("ascii_text"));

    auto* combo = panel.findChild<QComboBox*>(QStringLiteral("serialProtocolCombo"));
    QVERIFY(combo != nullptr);
    QCOMPARE(combo->count(), 2);
    QCOMPARE(combo->itemData(0).toString(), QStringLiteral("ascii_text"));
    QCOMPARE(combo->itemData(1).toString(), QStringLiteral("custom_md"));
    QCOMPARE(panel.activeProtocol(), QStringLiteral("ascii_text"));
}

void SerialProtocolPanelTest::setProtocolsDoesNotEmitSelection()
{
    SerialProtocolPanel panel;
    QSignalSpy selectedSpy(&panel, &SerialProtocolPanel::protocolSelected);

    panel.setProtocols({QStringLiteral("ascii_text"), QStringLiteral("custom_md")},
                       QStringLiteral("custom_md"));

    QCOMPARE(selectedSpy.count(), 0);
    QCOMPARE(panel.activeProtocol(), QStringLiteral("custom_md"));
}

void SerialProtocolPanelTest::missingActiveProtocolKeepsCurrentSelection()
{
    SerialProtocolPanel panel;
    panel.setProtocols({QStringLiteral("ascii_text"), QStringLiteral("custom_md")},
                       QStringLiteral("ascii_text"));

    panel.setActiveProtocol(QStringLiteral("missing_protocol"));

    QCOMPARE(panel.activeProtocol(), QStringLiteral("ascii_text"));
}

void SerialProtocolPanelTest::emptyProtocolListDisablesCombo()
{
    SerialProtocolPanel panel;
    panel.setProtocols({}, QString());

    auto* combo = panel.findChild<QComboBox*>(QStringLiteral("serialProtocolCombo"));
    QVERIFY(combo != nullptr);
    QCOMPARE(combo->count(), 0);
    QVERIFY(!combo->isEnabled());
    QCOMPARE(panel.activeProtocol(), QString());
}

void SerialProtocolPanelTest::invalidActivationDoesNotEmitSelection()
{
    SerialProtocolPanel panel;
    panel.setProtocols({QStringLiteral("ascii_text")}, QStringLiteral("ascii_text"));
    auto* combo = panel.findChild<QComboBox*>(QStringLiteral("serialProtocolCombo"));
    QVERIFY(combo != nullptr);

    QSignalSpy selectedSpy(&panel, &SerialProtocolPanel::protocolSelected);
    emit combo->activated(-1);

    QCOMPARE(selectedSpy.count(), 0);
    QCOMPARE(panel.activeProtocol(), QStringLiteral("ascii_text"));
}

QTEST_MAIN(SerialProtocolPanelTest)
#include "test_serial_protocol_panel.moc"
