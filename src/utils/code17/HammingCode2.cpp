/**
 * @file HammingCode2.cpp
 * @brief 扩展汉明码实现 — SEC-DED编解码/校验子/生成矩阵
 */

#include "utils/code17/HammingCode2.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
HammingCode2::HammingCode2(QObject* parent)
    : QObject(parent)
    , m_m(3)
    , m_timeSum(0.0)
{
}

/** @brief 设置码参数 @param m 校验位数(≥3) */
void HammingCode2::setCodeOrder(int m)
{
    m_m = qMax(3, m);
}

/** @brief 获取当前码参数 @return 码参数 */
HammingCode2::CodeParams HammingCode2::codeParams() const
{
    CodeParams p;
    p.m = m_m;
    p.n = (1 << m_m);          /* 扩展后: 2^m */
    p.k = (1 << m_m) - m_m - 1; /* 信息位 */
    p.dmin = 4;                /* SEC-DED最小距离为4 */
    return p;
}

/** @brief 编码 @param data 信息位 @return 码字(含扩展奇偶位) */
QBitArray HammingCode2::encode(const QBitArray& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = (1 << m_m);      /* 扩展码字长度 */
    int k = (1 << m_m) - m_m - 1;

    QBitArray codeword(n, false);

    /* 将信息位填入非校验位位置 */
    int dataIdx = 0;
    for (int i = 1; i < n; ++i) {
        if (!isPowerOfTwo(i)) {
            if (dataIdx < data.size() && dataIdx < k) {
                codeword[i - 1] = data[dataIdx];
            }
            ++dataIdx;
        }
    }

    /* 计算校验位 */
    for (int p = 0; p < m_m; ++p) {
        int parityPos = (1 << p);
        bool parity = false;
        for (int i = 1; i < n; ++i) {
            if (i & parityPos) {
                parity ^= codeword[i - 1];
            }
        }
        codeword[parityPos - 1] = parity;
    }

    /* 扩展奇偶位(全局奇偶校验) — 最后一位 */
    bool overallParity = computeOverallParity(codeword);
    codeword[n - 1] = overallParity;

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalEncoded;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncoded + m_stats.totalDecoded);

    emit encoded(k, n);
    return codeword;
}

/** @brief 解码并纠错 @param codeword 接收码字 @return 解码结果 */
HammingCode2::DecodeResult HammingCode2::decode(const QBitArray& codeword)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;
    int n = (1 << m_m);
    int k = (1 << m_m) - m_m - 1;

    if (codeword.size() < n) {
        result.success = false;
        result.errorsDetected = 1;
        return result;
    }

    /* 计算校验子 */
    QBitArray syndrome = computeSyndrome(codeword);
    int syndromeVal = 0;
    for (int i = 0; i < m_m; ++i) {
        if (syndrome[i]) syndromeVal |= (1 << i);
    }

    /* 计算全局奇偶校验 */
    bool receivedParity = codeword[n - 1];
    bool computedParity = false;
    for (int i = 0; i < n - 1; ++i) {
        computedParity ^= codeword[i];
    }
    bool parityError = (receivedParity != computedParity);

    result.correctedCodeword = codeword;

    if (syndromeVal == 0 && !parityError) {
        /* 无错误 */
        result.success = true;
        result.corrected = false;
        result.errorsDetected = 0;
    } else if (syndromeVal != 0 && parityError) {
        /* 单比特错误 — 可纠正 */
        int pos = syndromeVal;
        if (pos >= 1 && pos <= n) {
            result.correctedCodeword[pos - 1] = !result.correctedCodeword[pos - 1];
            result.corrected = true;
            result.errorPosition = pos - 1;
            result.errorsDetected = 1;
            result.success = true;
            ++m_stats.totalCorrections;
        } else {
            result.success = false;
            result.errorsDetected = 2;
            ++m_stats.totalUncorrectable;
        }
    } else if (syndromeVal == 0 && parityError) {
        /* 全局奇偶位错误 */
        result.correctedCodeword[n - 1] = !result.correctedCodeword[n - 1];
        result.corrected = true;
        result.errorPosition = n - 1;
        result.errorsDetected = 1;
        result.success = true;
        ++m_stats.totalCorrections;
    } else {
        /* 双比特错误 — 不可纠正 */
        result.success = false;
        result.errorsDetected = 2;
        ++m_stats.totalUncorrectable;
    }

    /* 提取信息位 */
    if (result.success) {
        /* (解码成功，信息位已从纠错后码字中隐含) */
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDecoded;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncoded + m_stats.totalDecoded);

    emit errorCorrected(result.errorPosition, result.corrected);
    return result;
}

/** @brief 从字节数组编码 @param bytes 原始数据 @return 编码后比特 */
QBitArray HammingCode2::encodeBytes(const QByteArray& bytes)
{
    int k = (1 << m_m) - m_m - 1;
    int n = (1 << m_m);
    int totalBits = bytes.size() * 8;

    /* 分块编码 */
    QVector<QBitArray> encodedBlocks;
    QBitArray allBits(totalBits);
    for (int i = 0; i < totalBits; ++i) {
        allBits[i] = (bytes[i / 8] >> (7 - (i % 8))) & 1;
    }

    int blocks = (totalBits + k - 1) / k;
    QBitArray result(blocks * n);
    int outIdx = 0;

    for (int b = 0; b < blocks; ++b) {
        QBitArray block(k, false);
        for (int i = 0; i < k; ++i) {
            int srcIdx = b * k + i;
            block[i] = (srcIdx < totalBits) ? allBits[srcIdx] : false;
        }
        QBitArray encoded = encode(block);
        for (int i = 0; i < n && outIdx < result.size(); ++i, ++outIdx) {
            result[outIdx] = encoded[i];
        }
    }

    return result;
}

/** @brief 解码到字节数组 @param bits 编码比特 @return (数据字节, 解码结果) */
QPair<QByteArray, HammingCode2::DecodeResult> HammingCode2::decodeToBytes(
    const QBitArray& bits)
{
    int k = (1 << m_m) - m_m - 1;
    int n = (1 << m_m);
    DecodeResult lastResult;
    QBitArray decodedBits((bits.size() / n) * k);
    int decIdx = 0;

    for (int b = 0; b + n <= bits.size(); b += n) {
        QBitArray block(n);
        for (int i = 0; i < n; ++i) block[i] = bits[b + i];

        DecodeResult res = decode(block);
        lastResult = res;

        if (res.success) {
            /* 提取信息位 */
            int infoIdx = 0;
            for (int i = 1; i < n; ++i) {
                if (!isPowerOfTwo(i) && i != n) {
                    if (decIdx < decodedBits.size()) {
                        decodedBits[decIdx++] = res.correctedCodeword[i - 1];
                    }
                    ++infoIdx;
                }
            }
        }
    }

    /* 比特转字节 */
    int byteCount = (decIdx + 7) / 8;
    QByteArray result(byteCount, 0);
    for (int i = 0; i < decIdx; ++i) {
        if (decodedBits[i]) {
            result[i / 8] |= (1 << (7 - (i % 8)));
        }
    }

    return {result, lastResult};
}

/** @brief 生成奇偶校验矩阵H @return H矩阵 */
QVector<QBitArray> HammingCode2::parityCheckMatrix() const
{
    int n = (1 << m_m) - 1;
    QVector<QBitArray> H(m_m);

    for (int col = 1; col <= n; ++col) {
        for (int row = 0; row < m_m; ++row) {
            if (!H[row].size()) H[row].resize(n);
            H[row][col - 1] = (col >> row) & 1;
        }
    }
    return H;
}

/** @brief 生成生成矩阵G @return G矩阵 */
QVector<QBitArray> HammingCode2::generatorMatrix() const
{
    int n = (1 << m_m) - 1;
    int k = n - m_m;

    /* G = [I_k | P^T] */
    QVector<QBitArray> G(k);
    for (int i = 0; i < k; ++i) {
        G[i].resize(n);
        G[i][i] = true; /* 单位矩阵部分 */
    }

    /* P矩阵: 非校验位置对应的校验方程 */
    auto H = parityCheckMatrix();
    int colIdx = 0;
    for (int j = 1; j <= n; ++j) {
        if (!isPowerOfTwo(j)) {
            for (int i = 0; i < m_m; ++i) {
                G[colIdx][n - m_m + i] = H[i][j - 1];
            }
            ++colIdx;
        }
    }

    return G;
}

/** @brief 构建校验子查找表 @return 校验子→错误位置映射 */
QMap<int, int> HammingCode2::syndromeTable() const
{
    QMap<int, int> table;
    int n = (1 << m_m);

    /* 无错误 */
    table[0] = -1;

    /* 单比特错误 */
    for (int pos = 1; pos < n; ++pos) {
        int syndrome = 0;
        for (int p = 0; p < m_m; ++p) {
            if (pos & (1 << p)) {
                syndrome |= (1 << p);
            }
        }
        table[syndrome] = pos - 1;
    }

    return table;
}

/** @brief 计算汉明距离 @param a 比特串a @param b 比特串b @return 汉明距离 */
int HammingCode2::hammingDistance(const QBitArray& a, const QBitArray& b)
{
    int minLen = qMin(a.size(), b.size());
    int dist = 0;
    for (int i = 0; i < minLen; ++i) {
        if (a[i] != b[i]) ++dist;
    }
    dist += qAbs(a.size() - b.size());
    return dist;
}

/** @brief 重置统计 */
void HammingCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 判断是否为2的幂 @param x 整数 @return 是否为2的幂 */
bool HammingCode2::isPowerOfTwo(int x) const
{
    return x > 0 && (x & (x - 1)) == 0;
}

/** @brief 校验子转错误位置 @param syndrome 校验子 @return 错误位置(0-based) */
int HammingCode2::syndromeToPosition(int syndrome) const
{
    if (syndrome == 0) return -1;
    return syndrome - 1;
}

/** @brief 计算校验子 @param codeword 码字 @return 校验子比特 */
QBitArray HammingCode2::computeSyndrome(const QBitArray& codeword) const
{
    QBitArray syndrome(m_m, false);
    int n = (1 << m_m);

    for (int i = 1; i < n; ++i) {
        if (i - 1 < codeword.size() && codeword[i - 1]) {
            for (int p = 0; p < m_m; ++p) {
                if (i & (1 << p)) {
                    syndrome[p] = !syndrome[p];
                }
            }
        }
    }
    return syndrome;
}

/** @brief 计算全局奇偶校验 @param bits 比特串 @return 偶校验结果 */
bool HammingCode2::computeOverallParity(const QBitArray& bits) const
{
    bool parity = false;
    for (int i = 0; i < bits.size(); ++i) {
        parity ^= bits[i];
    }
    return parity;
}
