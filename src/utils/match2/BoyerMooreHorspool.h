/**
 * @file BoyerMooreHorspool.h
 * @brief Boyer-Moore-Horspool字符串匹配引擎 — 坏字符跳跃快速搜索
 *
 * 实现BMH算法(Boyer-Moore的简化变体)，使用坏字符表实现O(nm)平均
 * 亚线性搜索性能。支持QString和QByteArray两种搜索目标，
 * 适用于协议帧定位、日志关键词搜索、二进制模式匹配等嵌入式调试场景。
 */
#pragma once

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVector>
#include <QElapsedTimer>

/**
 * @class BoyerMooreHorspool
 * @brief BMH字符串匹配引擎 — 坏字符跳跃 + 多模式搜索
 *
 * 典型用法:
 * @code
 *   BoyerMooreHorspool bmh;
 *   QVector<int> positions = bmh.searchAll(text, "pattern");
 * @endcode
 */
class BoyerMooreHorspool : public QObject {
    Q_OBJECT

public:
    /** @brief 搜索操作统计结构 */
    struct Stats {
        quint64 totalSearches   = 0;    ///< 搜索操作总次数
        quint64 totalMatches    = 0;    ///< 匹配命中的总次数(所有位置)
        double  avgProcessingTimeMs = 0.0; ///< 平均搜索耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit BoyerMooreHorspool(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~BoyerMooreHorspool() override;

    // ── 搜索接口 ──

    /**
     * @brief 在文本中搜索模式(返回第一个匹配位置)
     *
     * 使用BMH坏字符表进行跳跃，平均亚线性时间复杂度。
     * @param text    待搜索文本
     * @param pattern 搜索模式
     * @return 匹配位置列表(可能为空)
     */
    QVector<int> search(const QString& text, const QString& pattern);

    /**
     * @brief 在字节数组中搜索字节模式(返回第一个匹配位置)
     * @param data    待搜索数据
     * @param pattern 字节模式
     * @return 匹配位置列表(可能为空)
     */
    QVector<int> search(const QByteArray& data, const QByteArray& pattern);

    /**
     * @brief 搜索所有匹配位置
     *
     * 找到一个匹配后从下一位置继续搜索，收集所有匹配索引。
     * @param text    待搜索文本
     * @param pattern 搜索模式
     * @return 所有匹配起始位置(升序)
     */
    QVector<int> searchAll(const QString& text, const QString& pattern);

    /**
     * @brief 构建坏字符跳跃表
     *
     * 为BMH算法预计算跳跃距离表: 对于不在模式末尾的字符，
     * 跳跃距离 = 模式长度 - 1 - 该字符在模式中最右出现位置。
     * @param pattern 搜索模式
     * @return 跳跃距离表(按Unicode码点索引，大小256)
     */
    QVector<int> buildBadCharTable(const QString& pattern);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 @return Stats结构体副本 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 搜索完成信号 @param matchCount 匹配数量 @param searchTimeMs 搜索耗时(ms) */
    void searchCompleted(int matchCount, double searchTimeMs);

private:
    /**
     * @brief 为字节数组模式构建坏字符表
     * @param pattern 字节模式
     * @return 跳跃距离表(大小256)
     */
    QVector<int> buildByteBadCharTable(const QByteArray& pattern);

    Stats  m_stats;         ///< 操作统计
    QElapsedTimer m_timer;  ///< 处理耗时计时器
};
