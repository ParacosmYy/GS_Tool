/**
 * @file RegexEngine.h
 * @brief 正则表达式引擎 — Thompson NFA构造与匹配
 *
 * 功能: 实现基于Thompson NFA构造的正则表达式引擎，支持
 *       基本语法(连接/选择/闭包/括号/字符类)，将正则表达式
 *       编译为NFA后进行匹配。
 *
 * 协作: BytePatternAnalyzer(模式匹配) / ProtocolEngine(协议解析)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QSet>
#include <QChar>
#include <QString>
#include <QPair>

/**
 * @brief 正则表达式引擎 — Thompson NFA构造
 */
class RegexEngine : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        int totalCompilations = 0;          ///< 累计编译次数
        int totalMatches = 0;               ///< 累计匹配次数
        int totalSuccessfulMatches = 0;     ///< 累计成功匹配次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
        int totalNfaStatesCreated = 0;      ///< 累计NFA状态数
    };

    /** @brief NFA状态 */
    struct NfaState {
        int id = 0;                 ///< 状态编号
        QChar matchChar;            ///< 匹配字符(0=epsilon)
        bool isCharClass = false;   ///< 是否字符类
        QSet<QChar> charClass;      ///< 字符类集合
        bool negate = false;        ///< 字符类取反
        int out1 = -1;              ///< 转移目标1(-1=无)
        int out2 = -1;              ///< 转移目标2(-1=无,用于epsilon)
        bool isMatch = false;       ///< 是否接受状态
    };

    /** @brief 匹配结果 */
    struct MatchResult {
        bool matched = false;       ///< 是否匹配成功
        int start = -1;            ///< 匹配起始位置
        int length = 0;            ///< 匹配长度
        QString matchedText;       ///< 匹配的文本
    };

    explicit RegexEngine(QObject* parent = nullptr);

    /**
     * @brief 编译正则表达式
     * @param pattern 正则表达式字符串
     * @return 编译成功返回true
     */
    bool compile(const QString& pattern);

    /**
     * @brief 在文本中查找第一个匹配
     * @param text 输入文本
     * @return 匹配结果
     */
    MatchResult match(const QString& text) const;

    /**
     * @brief 在文本中查找所有匹配
     * @param text 输入文本
     * @return 所有匹配结果列表
     */
    QVector<MatchResult> matchAll(const QString& text);

    /**
     * @brief 测试文本是否完全匹配
     * @param text 输入文本
     * @return 匹配返回true
     */
    bool testMatch(const QString& text) const;

    /**
     * @brief 获取编译后的NFA状态列表
     * @return NFA状态列表(只读)
     */
    const QVector<NfaState>& nfaStates() const { return m_states; }

    /** @brief 是否已编译 @return 已编译返回true */
    bool isCompiled() const { return m_compiled; }

    /** @brief 获取最后编译的模式 @return 正则模式 */
    QString pattern() const { return m_pattern; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编译完成 @param success 是否成功 @param stateCount NFA状态数 */
    void compilationComplete(bool success, int stateCount);

    /** @brief 匹配完成 @param matched 是否匹配 @param count 匹配数 */
    void matchComplete(bool matched, int count);

private:
    /** @brief 解析正则表达式为中缀表示 @param pattern 正则串 @param pos 当前位置 @return 后缀位置 */
    int parseRegex(const QString& pattern, int pos);

    /** @brief 解析字符类 [a-z] @param pattern 正则串 @param pos 起始位置 @return 字符类和结束位置 */
    QPair<QSet<QChar>, int> parseCharClass(const QString& pattern, int pos);

    /** @brief 构建NFA片段 @param postfix 后缀表达式 @return 起始状态索引 */
    int buildNfa(const QVector<QPair<QString, QSet<QChar>>>& postfix);

    /** @brief epsilon闭包 @param states 当前状态集 @return 闭包状态集 */
    QSet<int> epsilonClosure(const QSet<int>& states) const;

    /** @brief 单步转移 @param states 当前状态集 @param c 输入字符 @return 转移后的状态集 */
    QSet<int> step(const QSet<int>& states, QChar c) const;

    /** @brief 在文本中从指定位置查找匹配 @param text 文本 @param start 起始位置 @return 匹配结果 */
    MatchResult findMatch(const QString& text, int start) const;

    /** @brief 添加NFA状态 @param state 状态 @return 状态索引 */
    int addState(const NfaState& state);

    QString m_pattern;              ///< 当前正则模式
    bool m_compiled = false;        ///< 是否已编译
    int m_startState = -1;          ///< NFA起始状态
    int m_acceptState = -1;         ///< NFA接受状态
    QVector<NfaState> m_states;     ///< NFA状态表
    mutable Stats m_stats;                  ///< 统计信息
    mutable double m_timeSum = 0.0;         ///< 累计耗时
};
