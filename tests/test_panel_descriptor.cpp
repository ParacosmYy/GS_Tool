#include <QtTest/QtTest>

#include "core/panels/PanelDescriptor.h"

class PanelDescriptorTest : public QObject {
    Q_OBJECT

private slots:
    void defaultDescriptorUsesWrappedPolicy();
    void descriptorStoresStableMetadata();
};

void PanelDescriptorTest::defaultDescriptorUsesWrappedPolicy()
{
    PanelDescriptor descriptor;

    QCOMPARE(descriptor.wrapperPolicy, PanelWrapperPolicy::Wrapped);
    QCOMPARE(descriptor.navOrder, -1);
    QCOMPARE(descriptor.stackOrder, -1);
    QVERIFY(descriptor.navVisible);
    QVERIFY(descriptor.includeInPanelStack);
    QCOMPARE(descriptor.rawWidget, nullptr);
}

void PanelDescriptorTest::descriptorStoresStableMetadata()
{
    PanelDescriptor descriptor;
    descriptor.id = "serial.config";
    descriptor.objectName = "serialConfigPanel";
    descriptor.groupKey = "连接";
    descriptor.titleKey = "配置";
    descriptor.iconName = "cable";
    descriptor.wrapperPolicy = PanelWrapperPolicy::RawPersistent;
    descriptor.navOrder = 7;
    descriptor.stackOrder = 3;
    descriptor.navVisible = false;
    descriptor.includeInPanelStack = false;

    QCOMPARE(QString::fromUtf8(descriptor.id), QStringLiteral("serial.config"));
    QCOMPARE(QString::fromUtf8(descriptor.objectName), QStringLiteral("serialConfigPanel"));
    QCOMPARE(QString::fromUtf8(descriptor.groupKey), QStringLiteral("连接"));
    QCOMPARE(QString::fromUtf8(descriptor.titleKey), QStringLiteral("配置"));
    QCOMPARE(QString::fromUtf8(descriptor.iconName), QStringLiteral("cable"));
    QCOMPARE(descriptor.wrapperPolicy, PanelWrapperPolicy::RawPersistent);
    QCOMPARE(descriptor.navOrder, 7);
    QCOMPARE(descriptor.stackOrder, 3);
    QVERIFY(!descriptor.navVisible);
    QVERIFY(!descriptor.includeInPanelStack);
}

QTEST_MAIN(PanelDescriptorTest)
#include "test_panel_descriptor.moc"
