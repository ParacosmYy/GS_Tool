#include <QtTest/QtTest>

#include <QPushButton>

#include "core/navigation/IconNavBar.h"
#include "core/navigation/NavigationController.h"

class IconNavBarTest : public QObject {
    Q_OBJECT

private slots:
    void categoryButtonUsesIconInsteadOfPlaceholderText();
    void categoriesFromMappingsPreserveFirstSeenOrder();
    void categoryTooltipShowsPanelCountAndCycleHint();
};

void IconNavBarTest::categoryButtonUsesIconInsteadOfPlaceholderText()
{
    IconNavBar navBar;

    NavCategory category;
    category.id = QStringLiteral("connection");
    category.iconName = QStringLiteral("cable");
    category.label = QStringLiteral("连接");
    category.panelIds = {QStringLiteral("serial.config")};

    navBar.setCategories({category});

    const auto buttons = navBar.findChildren<QPushButton*>();
    QCOMPARE(buttons.size(), 1);
    QVERIFY(!buttons.first()->icon().isNull());
    QCOMPARE(buttons.first()->text(), QString());
    QCOMPARE(buttons.first()->iconSize(), QSize(20, 20));
}

void IconNavBarTest::categoriesFromMappingsPreserveFirstSeenOrder()
{
    QWidget serialPanel;
    QWidget statsPanel;
    QWidget chartPanel;

    const auto categories = IconNavBar::categoriesFromMappings({
        {"终端", "终端", &statsPanel, "terminal.main", "terminal"},
        {"连接", "配置", &serialPanel, "serial.config", "cable"},
        {"终端", "统计", &statsPanel, "terminal.stats", "chart-no-axes-combined"},
        {"图表", "波形图", &chartPanel, "chart.main", "chart-line"},
    });

    QCOMPARE(categories.size(), 3);
    QCOMPARE(categories[0].id, QStringLiteral("终端"));
    QCOMPARE(categories[1].id, QStringLiteral("连接"));
    QCOMPARE(categories[2].id, QStringLiteral("图表"));
    QCOMPARE(categories[0].panelIds, QStringList({QStringLiteral("terminal.main"), QStringLiteral("terminal.stats")}));
    QCOMPARE(categories[1].panelIds, QStringList({QStringLiteral("serial.config")}));
    QCOMPARE(categories[0].iconName, QStringLiteral("terminal"));
}

void IconNavBarTest::categoryTooltipShowsPanelCountAndCycleHint()
{
    IconNavBar navBar;

    NavCategory category;
    category.id = QStringLiteral("terminal");
    category.iconName = QStringLiteral("terminal");
    category.label = QStringLiteral("终端");
    category.panelIds = {
        QStringLiteral("terminal.main"),
        QStringLiteral("terminal.stats")
    };

    navBar.setCategories({category});

    const auto buttons = navBar.findChildren<QPushButton*>();
    QCOMPARE(buttons.size(), 1);
    QCOMPARE(buttons.first()->toolTip(), QStringLiteral("终端 · 2 个面板\n再次点击切换下一个"));
}

QTEST_MAIN(IconNavBarTest)
#include "test_icon_nav_bar.moc"
