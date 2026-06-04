/**
 * @file TerminalTabManager.h
 * @brief 终端标签页管理控件
 *
 * 封装 QTabWidget，提供终端多标签页的增删和切换管理。
 */

#ifndef TERMINAL_TAB_MANAGER_H
#define TERMINAL_TAB_MANAGER_H

#include <QtGlobal>
#include <QWidget>

class QTabWidget;

/**
 * @class TerminalTabManager
 * @brief 终端多标签页管理器
 *
 * 每个标签页承载一个独立的终端实例，
 * 支持动态添加、移除标签页及当前页切换通知。
 */
class TerminalTabManager : public QWidget
{
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit TerminalTabManager(QWidget *parent = nullptr);

    /** @brief 析构函数 */
    ~TerminalTabManager() override;

    /**
     * @brief 添加一个新标签页
     * @param title 标签页标题
     * @return 新标签页的索引号
     */
    int addTab(const QString &title);

    /**
     * @brief 移除指定索引的标签页
     * @param index 要移除的标签页索引
     */
    void removeTab(int index);

    /**
     * @brief 获取标签页数量
     * @return 当前标签页总数
     */
    int tabCount() const;

    /**
     * @brief 获取当前选中标签页的索引
     * @return 当前标签页索引，无标签页时返回 -1
     */
    int currentTabIndex() const;

    /** @brief 获取标签页添加总次数 */
    quint64 totalTabAdds() const { return m_totalTabAdds; }

    /** @brief 获取标签页移除总次数 */
    quint64 totalTabRemoves() const { return m_totalTabRemoves; }

    /** @brief 获取标签页切换总次数 */
    quint64 totalTabSwitches() const { return m_totalTabSwitches; }

    /** @brief 获取历史峰值标签页数量 */
    quint64 peakTabCount() const { return m_peakTabCount; }

    /** @brief 获取累计标签页关闭前确认总次数(如有确认机制) */
    quint64 totalTabCloseRequests() const { return m_totalTabCloseRequests; }

    /** @brief 重置所有统计计数器 */
    void resetStatistics();

signals:
    /**
     * @brief 标签页被添加信号
     * @param index 新标签页的索引号
     */
    void tabAdded(int index);

    /**
     * @brief 标签页被移除信号
     * @param index 被移除标签页的索引号
     */
    void tabRemoved(int index);

    /**
     * @brief 当前标签页切换信号
     * @param index 新的当前标签页索引
     */
    void currentTabChanged(int index);

private:
    /** @brief 初始化界面布局 */
    void setupUI();

    QTabWidget *m_tabWidget; ///< 内部标签页控件

    quint64 m_totalTabAdds = 0;         ///< 标签页添加总次数
    quint64 m_totalTabRemoves = 0;      ///< 标签页移除总次数
    quint64 m_totalTabSwitches = 0;     ///< 标签页切换总次数
    quint64 m_peakTabCount = 0;         ///< 历史峰值标签页数量
    quint64 m_totalTabCloseRequests = 0;///< 累计标签页关闭请求总次数
};

#endif // TERMINAL_TAB_MANAGER_H
