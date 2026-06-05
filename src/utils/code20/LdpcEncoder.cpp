/**
 * @file LdpcEncoder.cpp
 * @brief LDPC系统编码器实现 — 校验矩阵构造/ALT/Richardson-Urbanke编码
 */

#include "utils/code20/LdpcEncoder.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/** @brief 构造函数 @param parent 父对象 */
LdpcEncoder::LdpcEncoder(QObject* parent)
    : QObject(parent)
    , m_method(ConstructionMethod::PEG)
    , m_timeSum(0.0)
{
}

/** @brief 初始化编码器 @param params LDPC参数 @param method 构造方法 @return 是否成功 */
bool LdpcEncoder::initialize(const LdpcParams& params, ConstructionMethod method)
{
    m_params = params;
    m_method = method;

    int N = params.blockLength;
    int K = params.messageLength;
    if (N <= 0 || K <= 0 || K >= N) return false;

    int M = N - K; /* 校验位数 */

    /* 构造校验矩阵 H (M x N) */
    m_H.clear();
    m_H.resize(M);
    for (int row = 0; row < M; ++row) {
        m_H[row].clear();
    }

    if (m_method == ConstructionMethod::Regular) {
        constructRegular();
    } else {
        constructPEG();
    }

    /* 构建ALT分解 */
    buildGeneratorMatrix();

    m_initialized = true;
    emit matrixConstructed(M, N);
    return true;
}

/** @brief 系统编码 @param message 信息比特 @return 码字 */
QVector<int> LdpcEncoder::encode(const QVector<int>& message)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeword;
    if (!m_initialized || message.size() != m_params.messageLength) {
        return codeword;
    }

    codeword = richardsonUrbankeEncode(message);

    /* 更新统计 */
    double elapsed = timer.elapsed();
    ++m_stats.totalEncodings;
    m_stats.totalBitsEncoded += m_params.blockLength;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncodings);

    emit encodingComplete(m_params.blockLength, 1);
    return codeword;
}

/** @brief 批量编码 @param messages 信息比特列表 @return 码字列表 */
QList<QVector<int>> LdpcEncoder::encodeBatch(const QList<QVector<int>>& messages)
{
    QList<QVector<int>> results;
    for (const auto& msg : messages) {
        results.append(encode(msg));
    }
    return results;
}

/** @brief 伴随式校验 @param codeword 码字 @return 是否通过 */
bool LdpcEncoder::syndromeCheck(const QVector<int>& codeword) const
{
    if (!m_initialized) return false;

    QVector<int> syndrome = matrixVectorMultiply(m_H, codeword);
    ++m_stats.totalSyndromeChecks;
    for (int s : syndrome) {
        if ((s & 1) != 0) return false;
    }
    return true;
}

/** @brief 获取校验矩阵 @return 稀疏表示 */
QList<QList<QPair<int, int>>> LdpcEncoder::parityCheckMatrix() const
{
    return m_H;
}

/** @brief 规则构造校验矩阵 */
void LdpcEncoder::constructRegular()
{
    int N = m_params.blockLength;
    int M = N - m_params.messageLength;
    int wc = m_params.columnWeight;
    int wr = qMax(1, N * wc / M);

    /* 逐列放置wc个1，分散到不同行 */
    std::mt19937 rng(42);
    for (int col = 0; col < N; ++col) {
        QVector<int> rows;
        for (int r = 0; r < M; ++r) rows.append(r);
        std::shuffle(rows.begin(), rows.end(), rng);

        int placed = 0;
        for (int r : rows) {
            if (placed >= wc) break;
            /* 检查行重限制 */
            if (m_H[r].size() < static_cast<size_t>(wr)) {
                m_H[r].append({col, 1});
                ++placed;
            }
        }
        /* 如果放不满，强制放 */
        if (placed < wc) {
            for (int r = 0; r < M && placed < wc; ++r) {
                bool exists = false;
                for (const auto& p : m_H[r]) {
                    if (p.first == col) { exists = true; break; }
                }
                if (!exists) {
                    m_H[r].append({col, 1});
                    ++placed;
                }
            }
        }
    }
}

/** @brief PEG构造校验矩阵 */
void LdpcEncoder::constructPEG()
{
    int N = m_params.blockLength;
    int M = N - m_params.messageLength;
    int wc = m_params.columnWeight;

    std::mt19937 rng(12345);

    /* 记录每个变量节点连接的校验节点 */
    QVector<QSet<int>> varEdges(N);

    for (int col = 0; col < N; ++col) {
        varEdges[col].clear();

        for (int e = 0; e < wc; ++e) {
            if (e == 0) {
                /* 第一条边: 选度数最低的校验节点 */
                int bestRow = 0;
                int minDeg = m_H[0].size();
                for (int r = 1; r < M; ++r) {
                    if (m_H[r].size() < static_cast<size_t>(minDeg)) {
                        minDeg = m_H[r].size();
                        bestRow = r;
                    }
                }
                m_H[bestRow].append({col, 1});
                varEdges[col].insert(bestRow);
            } else {
                /* 后续边: BFS找最远校验节点 */
                QSet<int> visited;
                QSet<int> currentLevel;
                for (int r : varEdges[col]) currentLevel.insert(r);

                int targetRow = -1;
                while (targetRow < 0) {
                    QSet<int> nextLevel;
                    for (int r : currentLevel) {
                        /* 遍历该校验节点连接的变量节点 */
                        for (const auto& pair : m_H[r]) {
                            int v = pair.first;
                            for (int cr : varEdges[v]) {
                                if (!visited.contains(cr)) {
                                    nextLevel.insert(cr);
                                }
                            }
                        }
                    }
                    visited.unite(currentLevel);

                    /* 从下一层选度数最低的 */
                    int minDeg = N + 1;
                    for (int r : nextLevel) {
                        if (!varEdges[col].contains(r) && m_H[r].size() < static_cast<size_t>(minDeg)) {
                            minDeg = m_H[r].size();
                            targetRow = r;
                        }
                    }
                    if (targetRow >= 0) break;
                    if (nextLevel.isEmpty()) {
                        /* 选任意未连接的校验节点 */
                        for (int r = 0; r < M; ++r) {
                            if (!varEdges[col].contains(r)) { targetRow = r; break; }
                        }
                        break;
                    }
                    currentLevel = nextLevel;
                }

                if (targetRow >= 0) {
                    m_H[targetRow].append({col, 1});
                    varEdges[col].insert(targetRow);
                }
            }
        }
    }
}

/** @brief 构建生成器矩阵(ALT分解) */
void LdpcEncoder::buildGeneratorMatrix()
{
    int N = m_params.blockLength;
    int M = N - m_params.messageLength;

    /* 简化ALT: 间隙g取较小值 */
    m_gap = qMin(10, M / 4);

    /* 将H分为 [A | B | C | D | E | T] 的简化形式 */
    int g = m_gap;
    int m = M - g;
    int K = m_params.messageLength;

    /* T: m x m 下三角 (校验位对应的校验部分) */
    m_T.clear();
    m_T.resize(m);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j <= i; ++j) {
            int srcRow = i;
            int srcCol = K + g + j;
            if (srcCol < N) {
                for (const auto& p : m_H[srcRow]) {
                    if (p.first == srcCol) {
                        m_T[i].append({j, p.second});
                        break;
                    }
                }
            }
        }
    }

    /* A: g x K (上方间隙部分) */
    m_A.clear();
    m_A.resize(g);
    for (int i = 0; i < g; ++i) {
        for (int j = 0; j < K; ++j) {
            for (const auto& p : m_H[i]) {
                if (p.first == j) {
                    m_A[i].append({j, p.second});
                    break;
                }
            }
        }
    }

    /* B: g x g */
    m_B.clear();
    m_B.resize(g);
    for (int i = 0; i < g; ++i) {
        for (int j = 0; j < g; ++j) {
            for (const auto& p : m_H[i]) {
                if (p.first == K + j) {
                    m_B[i].append({j, p.second});
                    break;
                }
            }
        }
    }

    /* D: m x K */
    m_D.clear();
    m_D.resize(m);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < K; ++j) {
            for (const auto& p : m_H[i]) {
                if (p.first == j) {
                    m_D[i].append({j, p.second});
                    break;
                }
            }
        }
    }

    /* E: m x g */
    m_E.clear();
    m_E.resize(m);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < g; ++j) {
            for (const auto& p : m_H[i]) {
                if (p.first == K + j) {
                    m_E[i].append({j, p.second});
                    break;
                }
            }
        }
    }

    m_C = m_T; /* 简化 */
}

/** @brief Richardson-Urbanke编码 @param msg 信息比特 @return 码字 */
QVector<int> LdpcEncoder::richardsonUrbankeEncode(const QVector<int>& msg) const
{
    int N = m_params.blockLength;
    int M = N - m_params.messageLength;
    int K = m_params.messageLength;

    QVector<int> codeword(N, 0);

    /* 系统部分: 前K位直接拷贝 */
    for (int i = 0; i < K; ++i) {
        codeword[i] = msg[i];
    }

    /* 计算校验位: 使用T的下三角结构进行前向替换 */
    /* p1 = T^{-1} * D * msg (模2) */
    QVector<int> dMsg = matrixVectorMultiply(m_D, msg);

    /* 前向替换求解 T*p1 = dMsg */
    int m = M - m_gap;
    QVector<int> p1(m, 0);
    for (int i = 0; i < m; ++i) {
        int sum = dMsg[i];
        for (const auto& pair : m_T[i]) {
            if (pair.first < i) sum += pair.second * p1[pair.first];
        }
        p1[i] = sum & 1;
    }

    /* 计算间隙校验位 p2 = A*msg + B*p1 的逆 */
    QVector<int> aMsg = matrixVectorMultiply(m_A, msg);
    QVector<int> bP1 = matrixVectorMultiply(m_B, p1);
    int g = m_gap;
    QVector<int> p2(g, 0);
    for (int i = 0; i < g; ++i) {
        p2[i] = (aMsg[i] + bP1[i]) & 1;
    }

    /* 组装码字 */
    for (int i = 0; i < g; ++i) codeword[K + i] = p2[i];
    for (int i = 0; i < m; ++i) codeword[K + g + i] = p1[i];

    return codeword;
}

/** @brief 稀疏矩阵-向量乘法(GF2) @param matrix 稀疏矩阵 @param vec 向量 @return 结果 */
QVector<int> LdpcEncoder::matrixVectorMultiply(
    const QList<QList<QPair<int, int>>>& matrix,
    const QVector<int>& vec) const
{
    int rows = matrix.size();
    QVector<int> result(rows, 0);
    for (int i = 0; i < rows; ++i) {
        int sum = 0;
        for (const auto& pair : matrix[i]) {
            sum += pair.second * vec[pair.first];
        }
        result[i] = sum & 1;
    }
    return result;
}

void LdpcEncoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
