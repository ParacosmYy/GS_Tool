/**
 * @file BookmarkWidget.h
 * @brief 书签管理面板 - 嵌入导航树面板，管理录制数据流中的时间点书签
 *
 * 设计思路:
 *   BookmarkWidget 是 DataBookmark Phase2 的 UI 组件，嵌入到导航树面板中，
 *   当用户选择"书签"导航项时显示。提供书签列表浏览、添加（弹出标签输入对话框）、
 *   删除选中项、清空全部、双击跳转等完整交互。
 *
 *   该控件仅负责表现层（UI 展示和用户交互），不直接操作 DataLogger。
 *   所有数据变更通过信号委托给上层（MainWindow 或未来控制器），由上层调用
 *   DataLogger 的 CRUD 接口，再通过 refreshBookmarks 槽刷新列表。
 *
 * 协作关系:
 *   - DataBookmark: 纯数据结构，BookmarkWidget 读取其 timestamp/label 字段显示
 *   - DataLogger: 持有书签集合，提供增删查接口（BookmarkWidget 不直接调用）
 *   - RecordingController: 拥有 addBookmarkRequested 信号，可转发来自此控件的书签请求
 *   - MainWindow: 连接 BookmarkWidget 的信号到 DataLogger 的槽，完成数据流闭环
 *
 * 设计模式:
 *   - 观察者模式: 通过 Qt 信号/槽通知上层书签操作请求
 *   - 委托模式: UI 操作委托为信号，不包含业务逻辑
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
 *
 * 列表项格式: "HH:mm:ss.zzz  标签文本"
 * 双击列表项发射 bookmarkDoubleClicked 信号，供未来跳转功能使用。
 */
class BookmarkWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief 构造书签管理面板
     * 初始化 UI 布局、按钮、列表控件，连接内部信号/槽
     * @param parent 父控件（通常为导航树面板中的 QStackedWidget 页面）
     */
    explicit BookmarkWidget(QWidget* parent = nullptr);

    /** @brief 禁止拷贝（QObject 派生类） */
    BookmarkWidget(const BookmarkWidget&) = delete;

    /** @brief 禁止赋值（QObject 派生类） */
    BookmarkWidget& operator=(const BookmarkWidget&) = delete;

signals:
    /**
     * @brief 请求添加书签
     * 用户在添加对话框中输入标签后发射，由上层连接到 DataLogger::addBookmark
     * @param label 用户输入的书签标签文本
     */
    void addBookmarkRequested(const QString& label);

    /**
     * @brief 请求删除指定索引的书签
     * 用户选中列表项并点击删除按钮后发射，由上层连接到 DataLogger::removeBookmark
     * @param index 要删除的书签在列表中的索引
     */
    void removeBookmarkRequested(int index);

    /**
     * @brief 请求清空所有书签
     * 用户点击清空按钮后发射，由上层连接到 DataLogger::clearBookmarks
     */
    void clearBookmarksRequested();

    /**
     * @brief 用户双击了某个书签项
     * 未来用于跳转到录制时间轴上对应的时间位置，当前仅发射信号
     * @param index 双击的书签在列表中的索引
     */
    void bookmarkDoubleClicked(int index);

public slots:
    /**
     * @brief 刷新书签列表显示
     * 由上层在书签数据变更后调用，用最新的书签集合替换当前列表内容。
     * 每个列表项的显示格式为: "HH:mm:ss.zzz  标签文本"
     * @param bookmarks 最新的书签集合（按添加顺序）
     */
    void refreshBookmarks(const QVector<DataBookmark>& bookmarks);

private slots:
    /** @brief 处理添加书签按钮点击，弹出标签输入对话框 */
    void onAddClicked();

    /** @brief 处理删除选中书签按钮点击，发射 removeBookmarkRequested */
    void onRemoveClicked();

    /** @brief 处理清空所有书签按钮点击，发射 clearBookmarksRequested */
    void onClearClicked();

    /**
     * @brief 处理列表项双击，发射 bookmarkDoubleClicked 信号
     * @param item 双击的列表项
     */
    void onItemDoubleClicked(QListWidgetItem* item);

private:
    /**
     * @brief 将书签时间戳格式化为可读字符串
     * 将 Unix 纪元毫秒时间戳转换为 "HH:mm:ss.zzz" 格式
     * @param timestampMs 毫秒级 Unix 时间戳
     * @return 格式化后的时间字符串
     */
    static QString formatTimestamp(qint64 timestampMs);

    /**
     * @brief 更新删除按钮和清空按钮的启用状态
     * 有选中项时启用删除按钮，列表非空时启用清空按钮
     */
    void updateButtonStates();

    // ---- UI 控件 ----

    /** @brief 标题标签，显示"书签列表" */
    QLabel* m_titleLabel = nullptr;

    /** @brief 书签列表控件，显示所有书签项 */
    QListWidget* m_listWidget = nullptr;

    /** @brief 添加书签按钮（objectName: bookmarkAddBtn） */
    QPushButton* m_addBtn = nullptr;

    /** @brief 删除选中书签按钮（objectName: bookmarkRemoveBtn） */
    QPushButton* m_removeBtn = nullptr;

    /** @brief 清空所有书签按钮（objectName: bookmarkClearBtn） */
    QPushButton* m_clearBtn = nullptr;
};

#endif // BOOKMARKWIDGET_H
