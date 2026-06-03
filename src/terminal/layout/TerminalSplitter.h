/**
 * @file TerminalSplitter.h
 * @brief 终端分栏布局控件
 *
 * 提供可动态增减的分栏视图，用于在同一界面中
 * 同时显示多个终端实例或不同的数据视图。
 */

#ifndef TERMINAL_SPLITTER_H
#define TERMINAL_SPLITTER_H

#include <QtGlobal>
#include <QWidget>

class QSplitter;

/**
 * @class TerminalSplitter
 * @brief 可动态增删分栏的终端布局控件
 *
 * 内部封装 QSplitter，支持水平/垂直分栏，
 * 可运行时动态添加或移除分栏区域。
 */
class TerminalSplitter : public QWidget
{
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit TerminalSplitter(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~TerminalSplitter() override;

    /**
     * @brief 设置分栏方向
     * @param orientation Qt::Horizontal 或 Qt::Vertical
     */
    void setOrientation(Qt::Orientation orientation);

    /**
     * @brief 添加一个新分栏区域
     * @return 新分栏的索引号
     */
    int addSection();

    /**
     * @brief 移除指定索引的分栏区域
     * @param index 要移除的分栏索引
     */
    void removeSection(int index);

    /**
     * @brief 获取当前分栏数量
     * @return 分栏数量
     */
    int sectionCount() const;

    /** @brief 获取分屏总次数 */
    quint64 totalSplits() const { return m_totalSplits; }

    /** @brief 获取合并总次数 */
    quint64 totalMerges() const { return m_totalMerges; }

    /**
     * @brief 获取分割条拖拽调整总次数
     * @return 用户拖拽调整分栏大小的累计次数
     */
    quint64 totalSplitResized() const { return m_totalSplitResized; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /**
     * @brief 分栏被添加信号
     * @param index 新分栏的索引号
     */
    void sectionAdded(int index);

    /**
     * @brief 分栏被移除信号
     * @param index 被移除分栏的索引号
     */
    void sectionRemoved(int index);

private:
    /** @brief 初始化界面布局 */
    void setupUI();

    QSplitter *m_splitter;      ///< 内部分栏控件
    int m_sectionCount = 0;     ///< 当前分栏计数

    quint64 m_totalSplits = 0;      ///< 分屏总次数
    quint64 m_totalMerges = 0;      ///< 合并总次数
    quint64 m_totalSplitResized = 0; ///< 分割条拖拽调整总次数
};

#endif // TERMINAL_SPLITTER_H
