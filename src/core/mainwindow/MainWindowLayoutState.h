/**
 * @file MainWindowLayoutState.h
 * @brief MainWindow导航区域splitter尺寸持久化辅助
 */

#ifndef MAINWINDOW_LAYOUT_STATE_H
#define MAINWINDOW_LAYOUT_STATE_H

#include <QList>

int savedNavTreeWidthFromSplitterSizes(const QList<int>& sizes, bool useIconNavBar);
QList<int> restoredNavigationSplitterSizes(int savedNavTreeWidth, int totalWidth, bool useIconNavBar);
bool navTreeIsCollapsedInSplitterSizes(const QList<int>& sizes, bool useIconNavBar);

#endif // MAINWINDOW_LAYOUT_STATE_H
