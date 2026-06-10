#include <QtTest/QtTest>

#include "core/navigation/NavigationController.h"

class NavPanelMappingTest : public QObject {
    Q_OBJECT

private slots:
    void mappingCarriesStableIdAndIconName();
};

void NavPanelMappingTest::mappingCarriesStableIdAndIconName()
{
    NavPanelMapping mapping{};
    mapping.id = "serial.config";
    mapping.category = "连接";
    mapping.name = "配置";
    mapping.iconName = "cable";

    QCOMPARE(QString::fromUtf8(mapping.id), QStringLiteral("serial.config"));
    QCOMPARE(QString::fromUtf8(mapping.iconName), QStringLiteral("cable"));
}

QTEST_GUILESS_MAIN(NavPanelMappingTest)
#include "test_nav_panel_mapping.moc"
