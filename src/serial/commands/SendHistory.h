/**
 * @file SendHistory.h
 * @brief 发送历史管理器 - 记录和查询发送命令历史，支持去重和搜索
 *
 * 职责:
 *   1. 维护最近N条发送命令的去重记录
 *   2. 提供关键词搜索能力（大小写不敏感）
 *   3. 为发送框下拉列表提供文本数据源
 *
 * 协作关系:
 *   - SendController: 每次发送成功后调用addEntry()记录
 *   - SendController: 通过recentTexts()获取自动补全列表
 */

#ifndef SENDHISTORY_H
#define SENDHISTORY_H

#include <QObject>
#include <QStringList>
#include <QDateTime>
#include <QList>
#include <QMap>

/**
 * @brief 发送历史记录条目
 */
struct SendEntry {
    QString text;       ///< 命令内容
    bool isHex;         ///< 是否HEX格式
    QDateTime time;     ///< 发送时间
};

/**
 * @brief 发送历史管理器 - 去重记录和搜索发送命令
 *
 * 自动去重：与最后一条内容相同时不添加，防止连续重复刷屏。
 * 最大记录数默认50，可通过setMaxEntries()调整。
 */
class SendHistory : public QObject {
    Q_OBJECT
public:
    /** @brief 构造发送历史管理器 @param parent 父对象 */
    explicit SendHistory(QObject* parent = nullptr);

    /**
     * @brief 添加一条发送记录
     * @param text 命令文本内容
     * @param isHex 是否为HEX格式
     *
     * 与最后一条内容相同时不添加（防止连续重复刷屏）
     */
    void addEntry(const QString& text, bool isHex);

    /**
     * @brief 获取最近的N条记录（纯文本列表，用于下拉框自动补全）
     * @param count 最大返回条数，默认50
     * @return 文本字符串列表
     */
    QStringList recentTexts(int count = 50) const;

    /** @brief 获取所有历史记录条目 */
    QList<SendEntry> entries() const;

    /**
     * @brief 搜索历史（按关键词过滤，大小写不敏感）
     * @param keyword 搜索关键词
     * @return 匹配的历史记录列表
     */
    QList<SendEntry> search(const QString& keyword) const;

    /** @brief 清空所有历史记录 */
    void clear();

    /**
     * @brief 设置最大记录数（默认50）
     * @param max 最大条目数
     */
    void setMaxEntries(int max);

    /** @brief 获取历史总发送次数（包括去重的） */
    int totalSendCount() const;

    /** @brief 获取最常发送的命令（按频率排序） */
    QList<QPair<QString, int>> mostFrequent(int topN = 10) const;

    /** @brief 获取发送统计摘要文本 */
    QString statisticsSummary() const;

    /** @brief 获取历史记录总条目数（累计添加，含去重跳过） */
    quint64 totalRecords() const;

    /** @brief 获取因连续重复而被跳过的去重次数 */
    quint64 totalDuplicateSkips() const;

    /** @brief 获取搜索调用总次数 */
    quint64 totalSearches() const;

    /** @brief 获取清空操作总次数 */
    quint64 totalClears() const;

    /** @brief 重置所有统计计数器（totalRecords/totalDuplicateSkips/totalSendCount/totalSearches/totalClears） */
    void resetStatistics();

signals:
    /** @brief 历史记录发生变更时发出（添加/清空） */
    void historyChanged();

private:
    QList<SendEntry> m_entries;             ///< 历史记录列表，按时间顺序排列
    QMap<QString, int> m_freqMap;           ///< 命令频率统计 text -> count
    int m_totalSendCount = 0;               ///< 总发送次数（含去重）
    int m_maxEntries = 50;                  ///< 最大记录条数
    quint64 m_totalRecords = 0;             ///< 历史记录总条目数（累计添加）
    quint64 m_totalDuplicateSkips = 0;      ///< 因连续重复而被跳过的去重次数
    mutable quint64 m_totalSearches = 0;    ///< 搜索调用总次数（mutable: const方法中递增）
    quint64 m_totalClears = 0;              ///< 清空操作总次数
};

#endif // SENDHISTORY_H
