/**
 * @file BytePatternAnalyzer.h
 * @brief 字节模式搜索器
 * @author Serial Tool Team
 * @date 2026-06-05
 *
 * 支持通配符(0x??)和掩码的灵活字节模式匹配引擎，
 * 可在串口数据流中批量搜索多个模式并导出CSV结果。
 */

#ifndef BYTEPATTERNANALYZER_H
#define BYTEPATTERNANALYZER_H

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

/**
 * @class BytePatternAnalyzer
 * @brief 字节模式搜索引擎
 *
 * 用户注册若干命名模式(支持通配符字节 0x?? 和掩码匹配)，
 * 之后对输入数据执行单模式搜索或批量全模式搜索。
 * 提供 CSV 导出和累计统计功能。
 */
class BytePatternAnalyzer : public QObject
{
    Q_OBJECT

public:
    /** @brief 命名搜索模式 */
    struct Pattern {
        QString name;           ///< 模式名称(用户自定义标识)
        QByteArray pattern;     ///< 模式字节序列(通配符位置填 0x00)
        QByteArray mask;        ///< 掩码: 0xFF=精确匹配该字节, 0x00=忽略该字节(通配)
    };

    /** @brief 单次搜索命中结果 */
    struct SearchResult {
        int offset = -1;           ///< 匹配在数据中的起始偏移量
        QString patternName;       ///< 命中的模式名称
        QByteArray matchedBytes;   ///< 实际匹配到的原始字节片段
    };

    /** @brief 累计统计数据 */
    struct Stats {
        quint64 totalSearches = 0;       ///< 累计搜索调用次数
        quint64 totalMatches = 0;        ///< 累计命中次数
        quint64 totalBytesScanned = 0;   ///< 累计扫描字节数
        quint64 patternsRegistered = 0;  ///< 当前已注册的模式数量
        quint64 searchTimeMs = 0;        ///< 累计搜索耗时(毫秒)
    };

    /** @brief 构造字节模式搜索器 @param parent 父对象 */
    explicit BytePatternAnalyzer(QObject *parent = nullptr);

    // ---- 模式管理 ----

    /** @brief 添加命名搜索模式
     *  @param name 模式名称(不可为空，不可重复)
     *  @param hexPattern 十六进制格式模式字符串，如 "AA BB ?? DD"
     *  @return true=添加成功，false=名称重复或格式无效
     */
    bool addPattern(const QString &name, const QString &hexPattern);

    /** @brief 移除指定名称的搜索模式 @param name 模式名称 @return true=移除成功 */
    bool removePattern(const QString &name);

    /** @brief 清除所有已注册模式 */
    void clearPatterns();

    /** @brief 获取所有已注册模式 @return 模式列表的只读引用 */
    const QList<Pattern> &patterns() const;

    // ---- 搜索 ----

    /** @brief 用指定模式搜索数据(单模式)
     *  @param data 待搜索数据
     *  @param patternName 模式名称
     *  @return 所有命中结果列表
     */
    QList<SearchResult> search(const QByteArray &data, const QString &patternName);

    /** @brief 批量搜索: 对数据依次执行多个模式搜索
     *  @param data 待搜索数据
     *  @param patternNames 模式名称列表
     *  @return 所有命中结果(按模式分组)
     */
    QList<SearchResult> search(const QByteArray &data, const QStringList &patternNames);

    /** @brief 全模式搜索: 用所有已注册模式对数据执行搜索
     *  @param data 待搜索数据
     *  @return 所有命中结果
     */
    QList<SearchResult> searchAll(const QByteArray &data);

    // ---- 导出 ----

    /** @brief 将搜索结果导出为CSV格式
     *  @param results 搜索结果列表
     *  @return CSV格式字符串(列: Offset,PatternName,HexMatch)
     */
    QString exportResults(const QList<SearchResult> &results) const;

    // ---- 统计 ----

    /** @brief 获取累计统计数据快照 @return Stats 结构体副本 */
    Stats stats() const;

    /** @brief 重置所有累计统计计数器 */
    void resetStatistics();

signals:
    /** @brief 搜索完成时发射 @param results 搜索命中结果 @param elapsedMs 本次搜索耗时(毫秒) */
    void searchCompleted(const QList<SearchResult> &results, qint64 elapsedMs);

    /** @brief 模式添加成功时发射 @param name 新增模式名称 */
    void patternAdded(const QString &name);

    /** @brief 模式移除成功时发射 @param name 移除的模式名称 */
    void patternRemoved(const QString &name);

private:
    /** @brief 将十六进制模式字符串解析为 Pattern 结构体
     *  @param hexPattern 十六进制字符串，如 "AA BB ?? DD"
     *  @return 解析后的 Pattern，解析失败返回空 pattern
     */
    Pattern parseHexPattern(const QString &name, const QString &hexPattern) const;

    /** @brief 对数据执行单模式匹配(核心匹配算法)
     *  @param data 待搜索数据
     *  @param pattern 模式结构体
     *  @return 所有命中偏移量列表
     */
    QList<int> matchPattern(const QByteArray &data, const Pattern &pattern) const;

    QList<Pattern> m_patterns;  ///< 已注册模式列表
    mutable Stats m_stats;      ///< 累计统计数据(mutable 允许 const 搜索更新统计)
};

#endif // BYTEPATTERNANALYZER_H
