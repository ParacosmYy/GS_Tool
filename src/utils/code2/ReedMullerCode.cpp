/**
 * @file ReedMullerCode.cpp
 * @brief Reed-Muller纠错码实现 — RM(r,m)编码/多数逻辑解码
 */

#include "utils/code2/ReedMullerCode.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param r 阶数 @param m 长度参数 @param parent 父对象 */
ReedMullerCode::ReedMullerCode(int r, int m, QObject* parent)
    : QObject(parent)
    , m_order(qMax(0, r))
    , m_lengthParam(qMax(1, m))
    , m_timeSum(0.0)
{
    m_n = 1 << m_lengthParam; /* 码长 = 2^m */

    /* 构造单项式列表和生成矩阵 */
    m_monomials = buildMonomials(m_lengthParam, m_order);
    m_generator = buildGeneratorMatrix(m_order, m_lengthParam);
    m_k = m_generator.size(); /* 消息长度 = 生成矩阵行数 */
}

/** @brief 编码 @param message 消息位向量 @return 码字 */
QVector<int> ReedMullerCode::encode(const QVector<int>& message)
{
    m_timer.start();

    QVector<int> result(m_n, 0);

    if (message.size() != m_k) {
        /* 消息长度不匹配，返回全零码字 */
        m_timeSum += m_timer.elapsed();
        ++m_stats.totalEncoded;
        m_stats.avgProcessingTimeMs = m_timeSum
            / static_cast<double>(m_stats.totalEncoded + m_stats.totalDecoded);
        emit encoded(0, m_n);
        return result;
    }

    /* 码字 = message * G (GF(2)矩阵乘法) */
    for (int i = 0; i < m_n; ++i) {
        int sum = 0;
        for (int j = 0; j < m_k; ++j) {
            sum ^= (message[j] & m_generator[j][i]); /* GF(2)乘加 */
        }
        result[i] = sum;
    }

    ++m_stats.totalEncoded;
    m_timeSum += m_timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncoded + m_stats.totalDecoded);

    emit encoded(m_k, m_n);
    return result;
}

/** @brief 解码(多数逻辑解码) @param received 接收码字 @return 消息 */
QVector<int> ReedMullerCode::decode(const QVector<int>& received)
{
    m_timer.start();

    if (received.size() != m_n) {
        m_timeSum += m_timer.elapsed();
        ++m_stats.totalDecoded;
        m_stats.avgProcessingTimeMs = m_timeSum
            / static_cast<double>(m_stats.totalEncoded + m_stats.totalDecoded);
        emit decoded(0);
        return QVector<int>(m_k, 0);
    }

    /* 使用接收码字的可变副本，逐层解码 */
    QVector<int> current = received;
    QVector<int> decodedMsg(m_k, 0);

    /* 从最高阶到最低阶解码 */
    int msgIdx = m_k - 1;

    for (int order = m_order; order >= 0; --order) {
        /* 获取当前阶数的单项式索引范围 */
        int startIdx = 0;
        for (int o = 0; o < order; ++o) {
            startIdx += binomial(m_lengthParam, o);
        }
        int endIdx = startIdx + binomial(m_lengthParam, order);

        for (int monoIdx = endIdx - 1; monoIdx >= startIdx; --monoIdx) {
            /* 构造该单项式的校验模式 */
            const QVector<int>& monomial = m_monomials[monoIdx];

            /* 使用多数逻辑: 枚举不在单项式中的变量的所有赋值 */
            /* 确定自由变量 */
            QVector<int> fixedVars = monomial;
            QVector<int> freeVars;
            for (int v = 0; v < m_lengthParam; ++v) {
                bool isFixed = false;
                for (int fv : fixedVars) {
                    if (fv == v) { isFixed = true; break; }
                }
                if (!isFixed) freeVars.append(v);
            }

            int numFree = freeVars.size();
            int numPatterns = 1 << numFree;
            int votesFor1 = 0;
            int votesFor0 = 0;

            for (int p = 0; p < numPatterns; ++p) {
                /* 构造完整赋值 */
                QVector<int> assign(m_lengthParam, 0);
                for (int v : fixedVars) assign[v] = 1;
                for (int fi = 0; fi < numFree; ++fi) {
                    assign[freeVars[fi]] = (p >> fi) & 1;
                }

                /* 将赋值映射到码字索引 */
                int codewordIdx = 0;
                for (int b = 0; b < m_lengthParam; ++b) {
                    codewordIdx |= (assign[b] << b);
                }

                if (current[codewordIdx] == 1) {
                    ++votesFor1;
                } else {
                    ++votesFor0;
                }
            }

            /* 多数判决 */
            decodedMsg[msgIdx] = (votesFor1 > votesFor0) ? 1 : 0;
            --msgIdx;

            /* 从当前码字中减去已解码分量 */
            if (decodedMsg[msgIdx + 1] == 1) {
                QVector<int> component(m_n, 0);
                for (int i = 0; i < m_n; ++i) {
                    component[i] = m_generator[monoIdx][i];
                }
                for (int i = 0; i < m_n; ++i) {
                    current[i] ^= component[i];
                }
            }
        }
    }

    /* 计算纠正的错误数 */
    QVector<int> reEncoded = encode(decodedMsg);
    int errors = 0;
    for (int i = 0; i < m_n; ++i) {
        if (received[i] != reEncoded[i]) ++errors;
    }

    ++m_stats.totalDecoded;
    m_timeSum += m_timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncoded + m_stats.totalDecoded);

    emit decoded(errors);
    return decodedMsg;
}

/** @brief 获取生成矩阵 @return 生成矩阵 */
QVector<QVector<int>> ReedMullerCode::generatorMatrix() const
{
    return m_generator;
}

/** @brief 计算最小距离 @return 最小汉明距离 */
int ReedMullerCode::minimumDistance() const
{
    return 1 << (m_lengthParam - m_order);
}

/** @brief 消息长度 @return k */
int ReedMullerCode::messageLength() const
{
    return m_k;
}

/** @brief 码字长度 @return n */
int ReedMullerCode::codewordLength() const
{
    return m_n;
}

/** @brief 重置统计 */
void ReedMullerCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 构造RM生成矩阵 @param r 阶数 @param m 长度参数 @return 生成矩阵 */
QVector<QVector<int>> ReedMullerCode::buildGeneratorMatrix(int r, int m)
{
    int n = 1 << m;
    QVector<QVector<int>> monomials = buildMonomials(m, r);
    int k = monomials.size();

    QVector<QVector<int>> G(k, QVector<int>(n, 0));

    for (int row = 0; row < k; ++row) {
        for (int col = 0; col < n; ++col) {
            /* 将列索引解释为变量赋值 */
            QVector<int> assign(m);
            for (int b = 0; b < m; ++b) {
                assign[b] = (col >> b) & 1;
            }
            G[row][col] = evaluateMonomial(monomials[row], assign);
        }
    }

    return G;
}

/** @brief 二项式系数 @param n 总数 @param k 选取数 @return C(n,k) */
int ReedMullerCode::binomial(int n, int k)
{
    if (k < 0 || k > n) return 0;
    if (k == 0 || k == n) return 1;
    if (k > n - k) k = n - k;

    qint64 result = 1;
    for (int i = 0; i < k; ++i) {
        result = result * (n - i) / (i + 1);
    }
    return static_cast<int>(result);
}

/** @brief GF(2)矩阵向量乘法 @param matrix 矩阵 @param vec 向量 @return 结果 */
QVector<int> ReedMullerCode::gf2Multiply(
    const QVector<QVector<int>>& matrix,
    const QVector<int>& vec)
{
    int rows = matrix.size();
    int cols = vec.size();
    QVector<int> result(rows, 0);

    for (int i = 0; i < rows; ++i) {
        int sum = 0;
        for (int j = 0; j < qMin(static_cast<int>(matrix[i].size()), cols); ++j) {
            sum ^= (matrix[i][j] & vec[j]);
        }
        result[i] = sum;
    }
    return result;
}

/** @brief 构造单项式列表 @param m 变量数 @param r 最大阶数 @return 单项式列表 */
QVector<QVector<int>> ReedMullerCode::buildMonomials(int m, int r)
{
    QVector<QVector<int>> monomials;

    /* 按阶数递增排列 */
    for (int order = 0; order <= r; ++order) {
        /* 枚举从m个变量中选order个的所有组合 */
        if (order == 0) {
            monomials.append({}); /* 常数1 */
            continue;
        }

        /* 使用迭代法枚举组合 */
        QVector<int> combo(order);
        for (int i = 0; i < order; ++i) {
            combo[i] = i;
        }

        while (true) {
            monomials.append(combo);

            /* 生成下一个组合 */
            int pos = order - 1;
            while (pos >= 0 && combo[pos] == m - order + pos) {
                --pos;
            }
            if (pos < 0) break;

            ++combo[pos];
            for (int i = pos + 1; i < order; ++i) {
                combo[i] = combo[i - 1] + 1;
            }
        }
    }

    return monomials;
}

/** @brief 评估单项式 @param monomial 变量索引 @param assignment 赋值 @return 0/1 */
int ReedMullerCode::evaluateMonomial(const QVector<int>& monomial,
                                      const QVector<int>& assignment)
{
    if (monomial.isEmpty()) return 1; /* 常数1 */

    int result = 1;
    for (int var : monomial) {
        result &= assignment[var];
        if (result == 0) break;
    }
    return result;
}
