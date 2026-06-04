/**
 * @file BytePatternAnalyzer.cpp
 * @brief 字节模式搜索器核心实现
 * @author Serial Tool Team
 * @date 2026-06-05
 *
 * 包含模式管理、十六进制解析、掩码匹配、批量搜索和 CSV 导出。
 */

#include "utils/pattern/BytePatternAnalyzer.h"

#include <QElapsedTimer>
#include <QStringList>
#include <QtGlobal>

// ──────────────────────────── 构造 ────────────────────────────

/** @brief 构造字节模式搜索器，设置 objectName 供 QSS 使用 @param parent 父对象 */
BytePatternAnalyzer::BytePatternAnalyzer(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("BytePatternAnalyzer"));
}

// ──────────────────────────── 模式管理 ────────────────────────────

/** @brief 添加命名搜索模式
 *
 * 将十六进制字符串(如 "AA BB ?? DD")解析为 Pattern 结构体，
 * 通配符 ?? 表示该字节位置匹配任意值。
 *
 * @param name 模式名称(不可为空，不可与已有模式重名)
 * @param hexPattern 十六进制格式模式字符串，支持空格/逗号分隔
 * @return true=添加成功，false=名称重复、为空或格式无效
 */
bool BytePatternAnalyzer::addPattern(const QString &name, const QString &hexPattern)
{
    if (name.isEmpty()) {
        return false;
    }

    /* 检查名称是否重复 */
    for (const auto &p : m_patterns) {
        if (p.name == name) {
            return false;
        }
    }

    Pattern parsed = parseHexPattern(name, hexPattern);
    if (parsed.pattern.isEmpty()) {
        return false;
    }

    m_patterns.append(parsed);
    m_stats.patternsRegistered = static_cast<quint64>(m_patterns.size());
    emit patternAdded(name);
    return true;
}

/** @brief 移除指定名称的搜索模式 @param name 模式名称 @return true=移除成功，false=名称不存在 */
bool BytePatternAnalyzer::removePattern(const QString &name)
{
    for (int i = 0; i < m_patterns.size(); ++i) {
        if (m_patterns[i].name == name) {
            m_patterns.removeAt(i);
            m_stats.patternsRegistered = static_cast<quint64>(m_patterns.size());
            emit patternRemoved(name);
            return true;
        }
    }
    return false;
}

/** @brief 清除所有已注册模式并更新统计 */
void BytePatternAnalyzer::clearPatterns()
{
    m_patterns.clear();
    m_stats.patternsRegistered = 0;
}

/** @brief 获取所有已注册模式 @return 模式列表的只读引用 */
const QList<BytePatternAnalyzer::Pattern> &BytePatternAnalyzer::patterns() const
{
    return m_patterns;
}

// ──────────────────────────── 搜索 ────────────────────────────

/** @brief 用指定模式搜索数据(单模式)
 *
 * 在 data 中查找与 patternName 对应的模式所有出现位置，
 * 使用掩码匹配: mask[i]==0xFF 时精确比较，mask[i]==0x00 时忽略。
 *
 * @param data 待搜索的数据缓冲区
 * @param patternName 已注册的模式名称
 * @return 所有命中结果列表，模式不存在或数据为空返回空列表
 */
QList<BytePatternAnalyzer::SearchResult> BytePatternAnalyzer::search(
    const QByteArray &data, const QString &patternName)
{
    QList<SearchResult> results;

    if (data.isEmpty()) {
        return results;
    }

    /* 查找对应模式 */
    const Pattern *target = nullptr;
    for (const auto &p : m_patterns) {
        if (p.name == patternName) {
            target = &p;
            break;
        }
    }
    if (!target) {
        return results;
    }

    QElapsedTimer timer;
    timer.start();

    QList<int> offsets = matchPattern(data, *target);

    /* 构造搜索结果 */
    for (int off : offsets) {
        SearchResult r;
        r.offset = off;
        r.patternName = patternName;
        r.matchedBytes = data.mid(off, target->pattern.size());
        results.append(r);
    }

    /* 更新统计 */
    m_stats.totalSearches += 1;
    m_stats.totalMatches += static_cast<quint64>(results.size());
    m_stats.totalBytesScanned += static_cast<quint64>(data.size());
    m_stats.searchTimeMs += static_cast<quint64>(timer.elapsed());

    emit searchCompleted(results, timer.elapsed());
    return results;
}

/** @brief 批量搜索: 对数据依次执行多个模式搜索
 *  @param data 待搜索数据
 *  @param patternNames 模式名称列表
 *  @return 所有命中结果(按模式输入顺序排列)
 */
QList<BytePatternAnalyzer::SearchResult> BytePatternAnalyzer::search(
    const QByteArray &data, const QStringList &patternNames)
{
    QList<SearchResult> allResults;
    for (const auto &name : patternNames) {
        allResults.append(search(data, name));
    }
    return allResults;
}

/** @brief 全模式搜索: 用所有已注册模式对数据执行搜索
 *
 * 等价于将 patterns() 中每个模式名称传入批量搜索。
 *
 * @param data 待搜索数据
 * @return 所有命中结果
 */
QList<BytePatternAnalyzer::SearchResult> BytePatternAnalyzer::searchAll(
    const QByteArray &data)
{
    QList<SearchResult> results;

    if (data.isEmpty() || m_patterns.isEmpty()) {
        return results;
    }

    QElapsedTimer timer;
    timer.start();

    for (const auto &pat : m_patterns) {
        QList<int> offsets = matchPattern(data, pat);
        for (int off : offsets) {
            SearchResult r;
            r.offset = off;
            r.patternName = pat.name;
            r.matchedBytes = data.mid(off, pat.pattern.size());
            results.append(r);
        }
    }

    /* 更新统计 */
    m_stats.totalSearches += static_cast<quint64>(m_patterns.size());
    m_stats.totalMatches += static_cast<quint64>(results.size());
    m_stats.totalBytesScanned += static_cast<quint64>(data.size()) * static_cast<quint64>(m_patterns.size());
    m_stats.searchTimeMs += static_cast<quint64>(timer.elapsed());

    emit searchCompleted(results, timer.elapsed());
    return results;
}

// ──────────────────────────── 导出 ────────────────────────────

/** @brief 将搜索结果导出为CSV格式
 *
 * 输出三列: Offset(十进制偏移), PatternName(模式名称), HexMatch(匹配字节十六进制)。
 * 第一行为表头，使用逗号分隔。
 *
 * @param results 搜索结果列表
 * @return CSV 格式字符串
 */
QString BytePatternAnalyzer::exportResults(const QList<SearchResult> &results) const
{
    QString csv = QStringLiteral("Offset,PatternName,HexMatch\n");

    for (const auto &r : results) {
        csv += QString::number(r.offset) + QStringLiteral(",")
             + r.patternName + QStringLiteral(",");

        /* 将匹配字节转为十六进制字符串 */
        QStringList hexBytes;
        for (char b : r.matchedBytes) {
            hexBytes << QStringLiteral("%1").arg(static_cast<quint8>(b), 2, 16, QChar('0')).toUpper();
        }
        csv += hexBytes.join(QStringLiteral(" ")) + QStringLiteral("\n");
    }

    return csv;
}

// ──────────────────────────── 内部方法 ────────────────────────────

/** @brief 将十六进制模式字符串解析为 Pattern 结构体
 *
 * 支持格式:
 *   - 空格分隔: "AA BB ?? DD"
 *   - 逗号分隔: "AA,BB,??,DD"
 *   - 无分隔符: "AABB??DD"(偶数长度)
 * 通配符 "??" 被转换为 mask 字节 0x00 和 pattern 字节 0x00。
 *
 * @param name 模式名称
 * @param hexPattern 十六进制输入字符串
 * @return 解析后的 Pattern，失败时 pattern 为空
 */
BytePatternAnalyzer::Pattern BytePatternAnalyzer::parseHexPattern(
    const QString &name, const QString &hexPattern) const
{
    Pattern result;
    result.name = name;

    if (hexPattern.isEmpty()) {
        return result;
    }

    /* 标准化: 统一替换逗号为空格，然后按空格分割 */
    QString normalized = hexPattern;
    normalized.replace(QLatin1Char(','), QLatin1Char(' '));

    QStringList tokens;
    /* 检测是否有空格分隔符 */
    if (normalized.contains(QLatin1Char(' '))) {
        tokens = normalized.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    } else if (normalized.size() % 2 == 0) {
        /* 无分隔符，每2个字符为一个token */
        for (int i = 0; i < normalized.size(); i += 2) {
            tokens << normalized.mid(i, 2);
        }
    } else {
        /* 奇数长度且无分隔符: 格式无效 */
        return result;
    }

    QByteArray patBytes;
    QByteArray maskBytes;

    for (const auto &token : tokens) {
        QString t = token.trimmed().toUpper();

        if (t == QStringLiteral("??") || t == QStringLiteral("**")) {
            /* 通配符: 匹配任意字节 */
            patBytes.append('\x00');
            maskBytes.append('\x00');
        } else {
            bool ok = false;
            quint8 val = static_cast<quint8>(t.toUInt(&ok, 16));
            if (!ok || t.length() > 2) {
                /* 解析失败: 非法十六进制 */
                return Pattern{};
            }
            patBytes.append(static_cast<char>(val));
            maskBytes.append('\xFF');
        }
    }

    result.pattern = patBytes;
    result.mask = maskBytes;
    return result;
}

/** @brief 对数据执行单模式匹配(核心掩码匹配算法)
 *
 * 对 data 中每个可能的起始位置 i，检查:
 *   (data[i+j] & mask[j]) == (pattern[j] & mask[j])
 * 当 mask[j]==0x00 时，左右两侧都为 0，条件恒成立(通配)。
 * 当 mask[j]==0xFF 时，退化为精确字节比较。
 *
 * @param data 待搜索数据
 * @param pattern 已解析的模式(含掩码)
 * @return 所有命中偏移量列表
 */
QList<int> BytePatternAnalyzer::matchPattern(
    const QByteArray &data, const Pattern &pattern) const
{
    QList<int> offsets;

    int patLen = pattern.pattern.size();
    int dataLen = data.size();

    if (patLen == 0 || patLen > dataLen) {
        return offsets;
    }

    const char *dataPtr = data.constData();
    const char *patPtr = pattern.pattern.constData();
    const char *maskPtr = pattern.mask.constData();

    int limit = dataLen - patLen;

    for (int i = 0; i <= limit; ++i) {
        bool matched = true;

        for (int j = 0; j < patLen; ++j) {
            unsigned char d = static_cast<unsigned char>(dataPtr[i + j]);
            unsigned char p = static_cast<unsigned char>(patPtr[j]);
            unsigned char m = static_cast<unsigned char>(maskPtr[j]);

            if ((d & m) != (p & m)) {
                matched = false;
                break;
            }
        }

        if (matched) {
            offsets.append(i);
        }
    }

    return offsets;
}
