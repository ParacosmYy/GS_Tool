/**
 * @file SendHistoryManager.h
 * @brief 发送历史管理器 - 封装自动补全基础设施和频率聚合逻辑
 *
 * 职责:
 *   1. 管理 QCompleter（基本补全）和 SmartAutoComplete（频率排序前缀匹配）
 *   2. 聚合历史条目的频率统计（按文本去重，累加频率，保留最近时间戳）
 *   3. 拦截输入框键盘事件用于补全列表导航（Up/Down/Enter/Escape）
 *   4. 记录发送历史并自动刷新补全数据源
 *
 * 设计模式:
 *   - 外观模式: 封装 SendHistory + QCompleter + SmartAutoComplete 的协作细节
 *
 * 协作关系:
 *   - SendHistory: 底层历史存储，提供 entries() 和 recentTexts() 数据源
 *   - SendController: 通过 recordHistory() 记录，通过 setupAutoComplete() 初始化
 *   - SmartAutoComplete: 频率排序前缀匹配弹出列表
 */

#ifndef SENDHISTORYMANAGER_H
#define SENDHISTORYMANAGER_H

#include <QObject>

class QLineEdit;
class QStringListModel;
class QCompleter;
class SendHistory;
class SmartAutoComplete;

/**
 * @brief 发送历史管理器 - 封装自动补全基础设施和频率聚合
 *
 * 对外暴露 setupAutoComplete()（绑定输入框）和 recordHistory()（记录历史），
 * 内部自动维护 QCompleter 和 SmartAutoComplete 的数据刷新。
 */
class SendHistoryManager : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 构造发送历史管理器
     * @param history 底层历史存储（外部拥有，不负责生命周期）
     * @param parent 父对象
     */
    explicit SendHistoryManager(SendHistory* history, QObject* parent = nullptr);

    /** @brief 析构函数，QObject 父子树自动回收子对象 */
    ~SendHistoryManager() override = default;

    /**
     * @brief 绑定自动补全到输入框
     *
     * 创建 QCompleter + SmartAutoComplete，安装事件过滤器，
     * 连接 historyChanged/textChanged/entrySelected 三组信号。
     * @param input 发送输入框
     * @param parentWidget 弹出列表的父 widget（用于定位）
     */
    void setupAutoComplete(QLineEdit* input, QWidget* parentWidget);

    /**
     * @brief 记录一条发送历史并自动刷新补全数据源
     * @param text 命令文本内容
     * @param isHex 是否为 HEX 格式
     */
    void recordHistory(const QString& text, bool isHex);

    /** @brief 获取智能补全实例（供外部查询状态） */
    SmartAutoComplete* smartComplete() const;

    // ---- 统计计数器接口 ----

    /** @brief 获取累计添加的历史记录数 @return 添加次数 */
    quint64 totalAdds() const;

    /** @brief 获取累计清空历史的次数 @return 清空次数 */
    quint64 totalClears() const;

    /** @brief 获取累计召回(补全选中)的次数 @return 召回次数 */
    quint64 totalRecalls() const;

    /** @brief 获取累计搜索/补全弹出次数 @return 搜索次数 */
    quint64 totalSearches() const;

    /** @brief 获取累计选中补全项次数(鼠标点击+键盘Enter) @return 选中次数 */
    quint64 totalSelects() const;

    /** @brief 获取历史列表的峰值大小(条目数) @return 峰值大小 */
    quint64 peakHistorySize() const;

    /** @brief 重置所有统计计数器(添加/清空/召回/搜索/选中/峰值) */
    void resetHistoryStatistics();

protected:
    /** @brief 事件过滤器 — 拦截输入框键盘事件用于补全列表导航 */
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    /** @brief 刷新 QCompleter 和 SmartAutoComplete 的数据源 */
    void refreshCompletions();

    SendHistory* m_sendHistory;             ///< 底层历史存储（外部拥有）
    QStringListModel* m_completerModel = nullptr; ///< 自动补全字符串列表模型
    QCompleter* m_completer = nullptr;      ///< 输入框基本补全器
    SmartAutoComplete* m_smartComplete = nullptr; ///< 智能补全弹出列表
    QLineEdit* m_input = nullptr;           ///< 绑定的输入框

    // 统计计数器
    quint64 m_totalAdds = 0;           ///< 累计添加的历史记录数
    quint64 m_totalClears = 0;         ///< 累计清空历史的次数
    quint64 m_totalRecalls = 0;        ///< 累计召回(补全选中)的次数
    quint64 m_totalSearches = 0;       ///< 累计搜索/补全弹出次数
    quint64 m_totalSelects = 0;        ///< 累计选中补全项次数(鼠标点击+键盘Enter)
    quint64 m_peakHistorySize = 0;     ///< 历史列表的峰值大小(条目数)
};

#endif // SENDHISTORYMANAGER_H
