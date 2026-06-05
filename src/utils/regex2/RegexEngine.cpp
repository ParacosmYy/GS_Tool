/**
 * @file RegexEngine.cpp
 * @brief 正则表达式引擎实现 — Thompson NFA构造与匹配
 */

#include "utils/regex2/RegexEngine.h"

#include <QElapsedTimer>
#include <QStack>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
RegexEngine::RegexEngine(QObject* parent)
    : QObject(parent)
{
}

/** @brief 编译正则表达式 @param pattern 正则模式 @return 成功返回true */
bool RegexEngine::compile(const QString& pattern)
{
    QElapsedTimer timer;
    timer.start();

    m_pattern = pattern;
    m_states.clear();
    m_compiled = false;

    if (pattern.isEmpty()) {
        emit compilationComplete(false, 0);
        return false;
    }

    /* 转换为中缀token列表 */
    QVector<QPair<QString, QSet<QChar>>> tokens;
    int pos = 0;
    while (pos < pattern.size()) {
        QChar c = pattern[pos];

        if (c == QLatin1Char('\\')) {
            /* 转义字符 */
            if (pos + 1 < pattern.size()) {
                QChar escaped = pattern[pos + 1];
                QSet<QChar> cs;
                cs.insert(escaped);
                tokens.append({QString(escaped), cs});
                pos += 2;
            } else {
                pos++;
            }
        } else if (c == QLatin1Char('[')) {
            /* 字符类 */
            auto result = parseCharClass(pattern, pos + 1);
            tokens.append({QString(), result.first});
            pos = result.second;
        } else if (c == QLatin1Char('.')) {
            /* 通配符(除换行外所有字符) */
            QSet<QChar> allChars;
            for (int i = 32; i < 127; ++i) {
                allChars.insert(static_cast<QChar>(i));
            }
            allChars.insert(QLatin1Char('\t'));
            tokens.append({QString(), allChars});
            pos++;
        } else if (c == QLatin1Char('*') || c == QLatin1Char('+')
                   || c == QLatin1Char('?') || c == QLatin1Char('|')
                   || c == QLatin1Char('(') || c == QLatin1Char(')')) {
            tokens.append({QString(c), {}});
            pos++;
        } else {
            /* 普通字符 */
            QSet<QChar> cs;
            cs.insert(c);
            tokens.append({QString(c), cs});
            pos++;
        }
    }

    /* 转换为后缀表达式(Shunting-yard) */
    QStack<QString> opStack;
    QVector<QPair<QString, QSet<QChar>>> postfix;
    int concatPending = 0;

    auto pushConcat = [&]() {
        while (concatPending > 0) {
            postfix.append({"CONCAT", {}});
            concatPending--;
        }
    };

    for (int i = 0; i < tokens.size(); ++i) {
        const auto& tok = tokens[i];
        QString type = tok.first;

        if (type == QLatin1String("|")) {
            pushConcat();
            opStack.push(type);
        } else if (type == QLatin1String("(")) {
            pushConcat();
            opStack.push(type);
        } else if (type == QLatin1String(")")) {
            pushConcat();
            while (!opStack.isEmpty() && opStack.top() != QLatin1String("(")) {
                postfix.append({opStack.pop(), {}});
            }
            if (!opStack.isEmpty()) opStack.pop();
            concatPending++;
        } else if (type == QLatin1String("*")
                   || type == QLatin1String("+")
                   || type == QLatin1String("?")) {
            postfix.append({type, {}});
            concatPending++;
        } else {
            postfix.append(tok);
            concatPending++;
        }
    }
    pushConcat();
    while (!opStack.isEmpty()) {
        postfix.append({opStack.pop(), {}});
    }

    /* 构建NFA */
    struct Fragment {
        int start;
        int end;
    };

    QStack<Fragment> fragStack;

    for (const auto& tok : postfix) {
        if (tok.first == QLatin1String("CONCAT")) {
            /* 连接: b跟在a后面 */
            if (fragStack.size() < 2) continue;
            Fragment b = fragStack.pop();
            Fragment a = fragStack.pop();
            /* a.end的epsilon指向b.start */
            m_states[a.end].out1 = b.start;
            fragStack.push({a.start, b.end});
        } else if (tok.first == QLatin1String("|")) {
            /* 选择 */
            if (fragStack.size() < 2) continue;
            Fragment b = fragStack.pop();
            Fragment a = fragStack.pop();
            int split = addState(NfaState{});
            m_states[split].out1 = a.start;
            m_states[split].out2 = b.start;
            int join = addState(NfaState{});
            m_states[a.end].out1 = join;
            m_states[b.end].out1 = join;
            fragStack.push({split, join});
        } else if (tok.first == QLatin1String("*")) {
            /* Kleene闭包 */
            if (fragStack.isEmpty()) continue;
            Fragment f = fragStack.pop();
            int split = addState(NfaState{});
            m_states[split].out1 = f.start;
            int join = addState(NfaState{});
            m_states[f.end].out1 = f.start;
            m_states[f.end].out2 = join;
            m_states[split].out2 = join;
            fragStack.push({split, join});
        } else if (tok.first == QLatin1String("+")) {
            /* 至少一次 */
            if (fragStack.isEmpty()) continue;
            Fragment f = fragStack.pop();
            int split = addState(NfaState{});
            m_states[split].out1 = f.start;
            int join = addState(NfaState{});
            m_states[f.end].out1 = f.start;
            m_states[f.end].out2 = join;
            fragStack.push({f.start, join});
        } else if (tok.first == QLatin1String("?")) {
            /* 零或一次 */
            if (fragStack.isEmpty()) continue;
            Fragment f = fragStack.pop();
            int split = addState(NfaState{});
            m_states[split].out1 = f.start;
            int join = addState(NfaState{});
            m_states[f.end].out1 = join;
            m_states[split].out2 = join;
            fragStack.push({split, join});
        } else {
            /* 字符/字符类 */
            int s = addState(NfaState{});
            int e = addState(NfaState{});
            m_states[s].out1 = e;
            if (!tok.second.isEmpty()) {
                m_states[s].isCharClass = true;
                m_states[s].charClass = tok.second;
            } else {
                m_states[s].matchChar = tok.first.isEmpty()
                    ? QChar() : tok.first[0];
            }
            fragStack.push({s, e});
        }
    }

    if (fragStack.isEmpty()) {
        emit compilationComplete(false, 0);
        return false;
    }

    Fragment mainFrag = fragStack.pop();
    m_startState = mainFrag.start;
    m_acceptState = mainFrag.end;
    m_states[m_acceptState].isMatch = true;

    m_compiled = true;
    m_stats.totalNfaStatesCreated += m_states.size();

    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    ++m_stats.totalCompilations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalCompilations + m_stats.totalMatches);

    emit compilationComplete(true, m_states.size());
    return true;
}

/** @brief 查找第一个匹配 @param text 输入文本 @return 匹配结果 */
RegexEngine::MatchResult RegexEngine::match(const QString& text) const
{
    if (!m_compiled) return MatchResult{};
    return findMatch(text, 0);
}

/** @brief 查找所有匹配 @param text 输入文本 @return 所有匹配 */
QVector<RegexEngine::MatchResult> RegexEngine::matchAll(const QString& text) const
{
    if (!m_compiled) return {};

    QElapsedTimer timer;
    timer.start();

    QVector<MatchResult> results;
    int pos = 0;
    while (pos < text.size()) {
        MatchResult mr = findMatch(text, pos);
        if (!mr.matched) break;
        results.append(mr);
        pos = mr.start + qMax(1, mr.length);
    }

    qint64 elapsed = timer.elapsed();
    const_cast<RegexEngine*>(this)->m_timeSum += static_cast<double>(elapsed);
    const_cast<RegexEngine*>(this)->m_stats.totalMatches += results.size();
    const_cast<RegexEngine*>(this)->m_stats.totalSuccessfulMatches += results.size();
    const_cast<RegexEngine*>(this)->m_stats.avgProcessingTimeMs = m_timeSum
        / (m_stats.totalCompilations + m_stats.totalMatches);

    emit matchComplete(!results.isEmpty(), results.size());
    return results;
}

/** @brief 测试是否完全匹配 @param text 文本 @return 匹配返回true */
bool RegexEngine::testMatch(const QString& text) const
{
    if (!m_compiled) return false;
    MatchResult mr = findMatch(text, 0);
    return mr.matched && mr.start == 0 && mr.length == text.size();
}

/** @brief 重置统计 */
void RegexEngine::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 解析字符类 @param pattern 正则串 @param pos 起始位置 @return 字符类和结束位置 */
QPair<QSet<QChar>, int> RegexEngine::parseCharClass(const QString& pattern, int pos)
{
    QSet<QChar> result;
    bool negate = false;

    if (pos < pattern.size() && pattern[pos] == QLatin1Char('^')) {
        negate = true;
        ++pos;
    }

    while (pos < pattern.size() && pattern[pos] != QLatin1Char(']')) {
        if (pos + 2 < pattern.size() && pattern[pos + 1] == QLatin1Char('-')
            && pattern[pos + 2] != QLatin1Char(']')) {
            /* 范围 a-z */
            QChar start = pattern[pos];
            QChar end = pattern[pos + 2];
            for (ushort c = start.unicode(); c <= end.unicode(); ++c) {
                result.insert(QChar(c));
            }
            pos += 3;
        } else {
            result.insert(pattern[pos]);
            ++pos;
        }
    }

    /* 跳过 ] */
    if (pos < pattern.size()) ++pos;

    if (negate) {
        QSet<QChar> complement;
        for (int c = 32; c < 127; ++c) {
            if (!result.contains(QChar(c))) {
                complement.insert(QChar(c));
            }
        }
        result = complement;
    }

    return {result, pos};
}

/** @brief epsilon闭包 @param states 状态集 @return 闭包 */
QSet<int> RegexEngine::epsilonClosure(const QSet<int>& states) const
{
    QSet<int> closure = states;
    QStack<int> stack;
    for (int s : states) stack.push(s);

    while (!stack.isEmpty()) {
        int s = stack.pop();
        if (s < 0 || s >= m_states.size()) continue;
        const NfaState& st = m_states[s];

        /* epsilon转移(无匹配字符且不是字符类) */
        if (!st.isCharClass && st.matchChar.isNull() && !st.isMatch) {
            if (st.out1 >= 0 && !closure.contains(st.out1)) {
                closure.insert(st.out1);
                stack.push(st.out1);
            }
            if (st.out2 >= 0 && !closure.contains(st.out2)) {
                closure.insert(st.out2);
                stack.push(st.out2);
            }
        }
    }
    return closure;
}

/** @brief 单步转移 @param states 状态集 @param c 字符 @return 转移后 */
QSet<int> RegexEngine::step(const QSet<int>& states, QChar c) const
{
    QSet<int> next;
    for (int s : states) {
        if (s < 0 || s >= m_states.size()) continue;
        const NfaState& st = m_states[s];
        bool matches = false;

        if (st.isCharClass) {
            matches = st.charClass.contains(c);
        } else if (!st.matchChar.isNull()) {
            matches = (st.matchChar == c);
        }

        if (matches && st.out1 >= 0) {
            next.insert(st.out1);
        }
    }
    return epsilonClosure(next);
}

/** @brief 查找匹配 @param text 文本 @param start 起始位置 @return 匹配结果 */
RegexEngine::MatchResult RegexEngine::findMatch(const QString& text, int start) const
{
    QElapsedTimer timer;
    timer.start();

    QSet<int> current = epsilonClosure({m_startState});
    MatchResult best;
    best.matched = false;

    for (int i = start; i <= text.size(); ++i) {
        /* 检查是否到达接受状态 */
        if (current.contains(m_acceptState)) {
            best.matched = true;
            best.start = start;
            best.length = i - start;
            best.matchedText = text.mid(start, best.length);
        }

        if (i >= text.size()) break;

        /* 转移 */
        current = step(current, text[i]);
        if (current.isEmpty()) break;
    }

    return best;
}

/** @brief 添加NFA状态 @param state 状态 @return 索引 */
int RegexEngine::addState(const NfaState& state)
{
    int idx = m_states.size();
    NfaState s = state;
    s.id = idx;
    m_states.append(s);
    return idx;
}
