/**
 * @file BookmarkWidget.h
 * @brief 书签管理面板 - 嵌入导航树面板，管理录制数据流中的时间点书签
 *
 * 设计思路:
 *   BookmarkWidget 是 DataBookmark Phase2 的 UI 组件，嵌入到导航树面板中，
 *   当用户选择"书签"导航项时显示。提供书签列表浏览、添加、删除、清空等交互。
 *
 *   该控件仅负责表现层（UI 展示和用户交互），不直接操作 DataLogger。
 *   所有数据变更通过信号委托给上层。
 *
 * 协作关系:
 *   - DataBookmark: 纯数据结构，BookmarkWidget 读取其 timestamp/label 字段显示
 *   - DataLogger: 持有书签集合，提供增删查接口
 *   - MainWindow: 连接信号到 DataLogger 的槽，完成数据流闭环
 */

#ifndef BOOKMARKWIDGET_H
#define BOOKMARKWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QLabel>
#include <QDateTime>

#include "utils/data/DataBookmark.h"

/**
 * @brief 书签管理面板 - 显示和管理录制时间轴上的书签标记
 *
 * 布局结构（从上到下）:
 *   1. 标题栏: "书签列表" 标签
 *   2. 工具栏: 添加按钮 | 删除按钮 | 清空按钮
 *   3. 书签列表: QListWidget 显示时间戳 + 标签
 */
class BookmarkWidget : public QWidget {
    Q_OBJECT

public:
    /** @brief 构造书签管理面板 */
    explicit BookmarkWidget(QWidget* parent = nullptr);

    BookmarkWidget(const BookmarkWidget&) = delete;
    BookmarkWidget& operator=(const BookmarkWidget&) = delete;

    /** @brief 获取累计添加书签次数 */
    quint64 totalBookmarksAdded() const;
    /** @brief 获取累计删除书签次数 */
    quint64 totalBookmarksRemoved() const;
    /** @brief 获取累计刷新次数 */
    quint64 totalRefreshes() const;
    /** @brief 获取累计导航（双击跳转）次数 */
    quint64 totalBookmarksNavigated() const;
    /** @brief 获取累计导入次数 @return 导入总次数 */
    quint64 totalImports() const;
    /** @brief 获取累计导出次数 @return 导出总次数 */
    quint64 totalExports() const;
    /** @brief 重置统计计数器 */
    void resetBookmarkStats();

signals:
    /** @brief 请求添加书签 */
    void addBookmarkRequested(const QString& label);
    /** @brief 请求删除指定索引的书签 */
    void removeBookmarkRequested(int index);
    /** @brief 请求清空所有书签 */
    void clearBookmarksRequested();
    /** @brief 用户双击了某个书签项 */
    void bookmarkDoubleClicked(int index);

public slots:
    /** @brief 刷新书签列表显示 */
    void refreshBookmarks(const QVector<DataBookmark>& bookmarks);

private slots:
    /** @brief 处理添加书签按钮点击 */
    void onAddClicked();
    /** @brief 处理删除选中书签按钮点击 */
    void onRemoveClicked();
    /** @brief 处理清空所有书签按钮点击 */
    void onClearClicked();
    /** @brief 处理列表项双击，发射bookmarkDoubleClicked信号 @param item 被双击的列表项 */
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    /** @brief 将毫秒时间戳格式化为 "HH:mm:ss.zzz" 字符串 @param timestampMs 毫秒时间戳 @return 格式化时间字符串 */
    static QString formatTimestamp(qint64 timestampMs);
    /** @brief 根据列表选中状态更新删除/清空按钮的启用状态 */
    void updateButtonStates();

    QLabel* m_titleLabel = nullptr;      ///< 标题标签
    QListWidget* m_listWidget = nullptr; ///< 书签列表控件
    QPushButton* m_addBtn = nullptr;     ///< 添加书签按钮
    QPushButton* m_removeBtn = nullptr;  ///< 删除选中书签按钮
    QPushButton* m_clearBtn = nullptr;   ///< 清空所有书签按钮

    quint64 m_totalBookmarksAdded = 0;      ///< 累计添加次数
    quint64 m_totalBookmarksRemoved = 0;   ///< 累计删除次数
    quint64 m_totalRefreshes = 0;          ///< 累计刷新次数
    quint64 m_totalBookmarksNavigated = 0; ///< 累计导航（双击跳转）次数
    quint64 m_totalImports = 0;            ///< 累计导入次数
    quint64 m_totalExports = 0;            ///< 累计导出次数
};

#endif // BOOKMARKWIDGET_H
