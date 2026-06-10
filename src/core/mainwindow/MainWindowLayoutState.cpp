/**
 * @file MainWindowLayoutState.cpp
 * @brief MainWindow导航区域splitter尺寸持久化辅助实现
 */

#include "core/mainwindow/MainWindowLayoutState.h"

#include <QtGlobal>

namespace {
constexpr int kIconNavBarWidth = 56;
constexpr int kContentMinWidth = 1;

int navTreeIndex(bool useIconNavBar)
{
    return useIconNavBar ? 1 : 0;
}
}

int savedNavTreeWidthFromSplitterSizes(const QList<int>& sizes, bool useIconNavBar)
{
    const int index = navTreeIndex(useIconNavBar);
    if (sizes.size() <= index) {
        return 0;
    }

    return sizes.at(index);
}

QList<int> restoredNavigationSplitterSizes(int savedNavTreeWidth, int totalWidth, bool useIconNavBar)
{
    if (savedNavTreeWidth <= 0) {
        return {};
    }

    if (useIconNavBar) {
        const int maxTreeWidth = qMax(0, totalWidth - kIconNavBarWidth - kContentMinWidth);
        const int navTreeWidth = qMin(savedNavTreeWidth, maxTreeWidth);
        return {kIconNavBarWidth, navTreeWidth, totalWidth - navTreeWidth - kIconNavBarWidth};
    }

    const int maxTreeWidth = qMax(0, totalWidth - kContentMinWidth);
    const int navTreeWidth = qMin(savedNavTreeWidth, maxTreeWidth);
    return {navTreeWidth, totalWidth - navTreeWidth};
}

bool navTreeIsCollapsedInSplitterSizes(const QList<int>& sizes, bool useIconNavBar)
{
    return savedNavTreeWidthFromSplitterSizes(sizes, useIconNavBar) <= 0;
}
