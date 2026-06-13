#include <QtTest/QtTest>

#include <QStandardItemModel>
#include <QTreeView>
#include <QWidget>

#include "core/navigation/NavigationController.h"

class NavigationControllerCategoryTest : public QObject {
    Q_OBJECT

private slots:
    void findsFirstPanelByCategoryKey();
    void findsCategoryForPanel();
    void emitsCurrentPanelChangedWhenPanelChanges();
    void findsNextPanelWithinCategory();
    void selectsNavTreeItemForCurrentPanel();
    void selectsNavTreeItemForInterleavedCategoryMapping();
    void clickingNavTreeLeafSwitchesCurrentPanel();
    void restoresPanelByStableId();
    void unknownStableIdDoesNotChangeCurrentPanel();
};

void NavigationControllerCategoryTest::findsFirstPanelByCategoryKey()
{
    NavigationController controller;
    QWidget serialPanel;
    QWidget blePanel;
    QWidget chartPanel;
    QTreeView navTree;

    controller.buildNavTree(&navTree, {
        {"连接", "配置", &serialPanel, "serial.config", "cable"},
        {"连接", "BLE配置", &blePanel, "connection.ble.config", "bluetooth"},
        {"图表", "波形图", &chartPanel, "chart.main", "chart-line"},
    });

    QCOMPARE(controller.firstPanelInCategory(QStringLiteral("连接")), &serialPanel);
    QCOMPARE(controller.firstPanelInCategory(QStringLiteral("图表")), &chartPanel);
    QCOMPARE(controller.firstPanelInCategory(QStringLiteral("missing")), nullptr);
}

void NavigationControllerCategoryTest::findsCategoryForPanel()
{
    NavigationController controller;
    QWidget serialPanel;
    QWidget chartPanel;
    QTreeView navTree;

    controller.buildNavTree(&navTree, {
        {"连接", "配置", &serialPanel, "serial.config", "cable"},
        {"图表", "波形图", &chartPanel, "chart.main", "chart-line"},
    });

    QCOMPARE(controller.categoryForPanel(&serialPanel), QStringLiteral("连接"));
    QCOMPARE(controller.categoryForPanel(&chartPanel), QStringLiteral("图表"));
    QCOMPARE(controller.categoryForPanel(nullptr), QString());
}

void NavigationControllerCategoryTest::emitsCurrentPanelChangedWhenPanelChanges()
{
    NavigationController controller;
    QWidget serialPanel;
    QWidget chartPanel;
    QTreeView navTree;

    controller.buildNavTree(&navTree, {
        {"连接", "配置", &serialPanel, "serial.config", "cable"},
        {"图表", "波形图", &chartPanel, "chart.main", "chart-line"},
    });
    controller.setCurrentPanel(&serialPanel);

    qRegisterMetaType<QWidget*>("QWidget*");
    QSignalSpy spy(&controller, &NavigationController::currentPanelChanged);
    controller.switchToPanel(&chartPanel);

    QCOMPARE(spy.count(), 1);
    QCOMPARE(qvariant_cast<QWidget*>(spy.first().first()), &chartPanel);
}

void NavigationControllerCategoryTest::findsNextPanelWithinCategory()
{
    NavigationController controller;
    QWidget serialPanel;
    QWidget blePanel;
    QWidget chartPanel;
    QTreeView navTree;

    controller.buildNavTree(&navTree, {
        {"连接", "配置", &serialPanel, "serial.config", "cable"},
        {"连接", "BLE配置", &blePanel, "connection.ble.config", "bluetooth"},
        {"图表", "波形图", &chartPanel, "chart.main", "chart-line"},
    });

    controller.setCurrentPanel(&serialPanel);
    QCOMPARE(controller.nextPanelInCategory(QStringLiteral("连接")), &blePanel);

    controller.setCurrentPanel(&blePanel);
    QCOMPARE(controller.nextPanelInCategory(QStringLiteral("连接")), &serialPanel);

    controller.setCurrentPanel(&chartPanel);
    QCOMPARE(controller.nextPanelInCategory(QStringLiteral("连接")), &serialPanel);
}

void NavigationControllerCategoryTest::selectsNavTreeItemForCurrentPanel()
{
    NavigationController controller;
    QWidget serialPanel;
    QWidget chartPanel;
    QTreeView navTree;

    controller.buildNavTree(&navTree, {
        {"连接", "配置", &serialPanel, "serial.config", "cable"},
        {"图表", "波形图", &chartPanel, "chart.main", "chart-line"},
    });

    controller.setCurrentPanel(&chartPanel);

    QVERIFY(navTree.currentIndex().isValid());
    QCOMPARE(navTree.currentIndex().data().toString(), QStringLiteral("波形图"));
}

void NavigationControllerCategoryTest::selectsNavTreeItemForInterleavedCategoryMapping()
{
    NavigationController controller;
    QWidget serialPanel;
    QWidget chartPanel;
    QWidget blePanel;
    QTreeView navTree;

    controller.buildNavTree(&navTree, {
        {"连接", "配置", &serialPanel, "serial.config", "cable"},
        {"图表", "波形图", &chartPanel, "chart.main", "chart-line"},
        {"连接", "BLE配置", &blePanel, "connection.ble.config", "bluetooth"},
    });

    controller.setCurrentPanel(&blePanel);

    QVERIFY(navTree.currentIndex().isValid());
    QCOMPARE(navTree.currentIndex().data().toString(), QStringLiteral("BLE配置"));
}

void NavigationControllerCategoryTest::clickingNavTreeLeafSwitchesCurrentPanel()
{
    NavigationController controller;
    QWidget serialPanel;
    QWidget chartPanel;
    QTreeView navTree;

    controller.buildNavTree(&navTree, {
        {"连接", "配置", &serialPanel, "serial.config", "cable"},
        {"图表", "波形图", &chartPanel, "chart.main", "chart-line"},
    });
    controller.setCurrentPanel(&serialPanel);

    auto* model = qobject_cast<QStandardItemModel*>(navTree.model());
    QVERIFY(model);
    const QModelIndex chartCategoryIndex = model->index(1, 0);
    const QModelIndex chartPanelIndex = model->index(0, 0, chartCategoryIndex);
    QVERIFY(chartPanelIndex.isValid());

    qRegisterMetaType<QWidget*>("QWidget*");
    QSignalSpy spy(&controller, &NavigationController::currentPanelChanged);
    QVERIFY(QMetaObject::invokeMethod(&navTree, "clicked", Qt::DirectConnection,
                                      Q_ARG(QModelIndex, chartPanelIndex)));

    QCOMPARE(controller.currentPanelIndex(), 1);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(qvariant_cast<QWidget*>(spy.first().first()), &chartPanel);
}

void NavigationControllerCategoryTest::restoresPanelByStableId()
{
    NavigationController controller;
    QWidget serialPanel;
    QWidget stationPanel;
    QTreeView navTree;

    controller.buildNavTree(&navTree, {
        {"连接", "配置", &serialPanel, "serial.config", "cable"},
        {"连接", "串口工站", &stationPanel, "serial.station", "terminal"},
    });

    QVERIFY(controller.restorePanelById(QStringLiteral("serial.station")));
    QCOMPARE(controller.currentPanelIndex(), 1);
    QVERIFY(stationPanel.isVisible());
    QVERIFY(!serialPanel.isVisible());
    QVERIFY(navTree.currentIndex().isValid());
    QCOMPARE(navTree.currentIndex().data().toString(), QStringLiteral("串口工站"));
}

void NavigationControllerCategoryTest::unknownStableIdDoesNotChangeCurrentPanel()
{
    NavigationController controller;
    QWidget serialPanel;
    QWidget stationPanel;
    QTreeView navTree;

    controller.buildNavTree(&navTree, {
        {"连接", "配置", &serialPanel, "serial.config", "cable"},
        {"连接", "串口工站", &stationPanel, "serial.station", "terminal"},
    });
    QVERIFY(controller.restorePanelById(QStringLiteral("serial.config")));

    QVERIFY(!controller.restorePanelById(QStringLiteral("missing.station")));
    QCOMPARE(controller.currentPanelIndex(), 0);
    QVERIFY(serialPanel.isVisible());
    QVERIFY(!stationPanel.isVisible());
}

QTEST_MAIN(NavigationControllerCategoryTest)
#include "test_navigation_controller_category.moc"
