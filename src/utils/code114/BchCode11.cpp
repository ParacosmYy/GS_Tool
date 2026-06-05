#include "BchCode11.h"
#include <QElapsedTimer>
#include <QSet>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化BCH编解码器
 * @param parent 父对象指针
 */
BchCode11::BchCode11(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void BchCode11::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief GF(2)上多项式乘法辅助函数
 *
 * 二进制系数多项式在GF(2)上的乘法，系数异或运算。
 *
 * @param a 第一个多项式系数
 * @param b 第二个多项式系数
 * @return 乘积多项式系数
 */
static QVector<int> gf2PolyMul(const QVector<int>& a, const QVector<int>& b)
{
    if (a.isEmpty() || b.isEmpty()) return {0};
    QVector<int> result(a.size() + b.size() - 1, 0);
    for (int i = 0; i < a.size(); ++i) {
        if (a[i] == 0) continue;
        for (int j = 0; j < b.size(); ++j) {
            result[i + j] ^= b[j];
        }
    }
    return result;
}

/**
 * @brief GF(2)上多项式除法求余数
 *
 * 二进制系数多项式长除法，返回余数多项式。
 * 用于BCH编码中计算校验位。
 *
 * @param dividend 被除式
 * @param divisor 除式
 * @return 余式多项式系数
 */
static QVector<int> gf2PolyMod(const QVector<int>& dividend, const QVector<int>& divisor)
{
    if (divisor.isEmpty()) return dividend;
    QVector<int> rem = dividend;
    int degDiv = divisor.size() - 1;
    /* 找到除式最高非零系数 */
    while (degDiv >= 0 && divisor[degDiv] == 0) degDiv--;

    for (int i = rem.size() - 1; i >= degDiv; --i) {
        if (rem[i] != 0) {
            for (int j = 0; j <= degDiv; ++j) {
                if (divisor[j] != 0) {
                    int idx = i - degDiv + j;
                    if (idx >= 0 && idx < rem.size()) {
                        rem[idx] ^= 1;
                    }
                }
            }
        }
    }
    /* 截取余数部分 */
    int remLen = degDiv;
    rem.resize(qMax(1, remLen));
    return rem;
}

/**
 * @brief 生成BCH码的生成多项式
 *
 * BCH码生成多项式g(x)是α, α^3, ..., α^{2t-1}
 * 的最小多项式的最小公倍式(LCM)。
 * 在GF(2)上计算每个根的最小多项式m_i(x)，
 * 然后依次相乘得到g(x)。
 *
 * @param codeLength 码长n（通常为2^m - 1）
 * @param errorCorrectionT 纠错能力t
 * @return 生成多项式系数（从低次到高次）
 */
QVector<int> BchCode11::generatePolynomial(int codeLength, int errorCorrectionT)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> genPoly = {1};
    if (codeLength <= 0 || errorCorrectionT <= 0) {
        emit decodeCompleted(0);
        return genPoly;
    }

    /* 确定m值: n <= 2^m - 1 */
    int m = 1;
    while ((1 << m) - 1 < codeLength && m < 20) m++;

    /* 依次计算 α^{2i-1} 的最小多项式并求LCM */
    for (int i = 1; i <= errorCorrectionT; ++i) {
        int rootExp = 2 * i - 1; /* 奇数次幂 α^1, α^3, α^5, ... */

        /* 计算最小多项式: (x - α^e)(x - α^{2e})...(x - α^{2^{m-1}e}) */
        QVector<int> minPoly = {1};
        int conjugateExp = rootExp;
        QSet<int> usedExps;

        for (int c = 0; c < m; ++c) {
            int exp = conjugateExp % ((1 << m) - 1);
            if (!usedExps.contains(exp)) {
                usedExps.insert(exp);
                /* 乘以 (x + α^exp)，用不可约多项式模运算 */
                QVector<int> factor = {exp, 1}; /* α^exp + x */
                minPoly = gf2PolyMul(minPoly, factor);
                /* 将系数折叠为GF(2) */
                for (auto& coef : minPoly) {
                    coef = coef & 1;
                }
            }
            conjugateExp = (conjugateExp * 2) % ((1 << m) - 1);
        }

        /* 将最小多项式系数二值化 */
        for (auto& c : minPoly) c = c & 1;

        /* 乘入生成多项式 */
        genPoly = gf2PolyMul(genPoly, minPoly);
        for (auto& c : genPoly) c = c & 1;
    }

    /* 去除前导零 */
    while (genPoly.size() > 1 && genPoly.last() == 0) {
        genPoly.removeLast();
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(0);
    return genPoly;
}

/**
 * @brief BCH编码
 *
 * 系统编码：码字c(x) = x^{n-k} * m(x) + r(x)，
 * 其中r(x) = x^{n-k} * m(x) mod g(x)为校验多项式。
 * 消息位在高位，校验位在低位。
 *
 * @param message 原始消息比特序列
 * @param generatorPoly 生成多项式系数
 * @return 编码码字[消息位 | 校验位]
 */
QVector<int> BchCode11::encode(const QVector<int>& message, const QVector<int>& generatorPoly)
{
    QElapsedTimer timer;
    timer.start();

    if (message.isEmpty() || generatorPoly.isEmpty()) {
        emit decodeCompleted(0);
        return message;
    }

    int genDeg = generatorPoly.size() - 1;

    /* 消息移位: x^{n-k} * m(x) */
    QVector<int> shifted(message.size() + genDeg, 0);
    for (int i = 0; i < message.size(); ++i) {
        shifted[i] = message[i] & 1;
    }

    /* 多项式除法求余数 */
    QVector<int> remainder = gf2PolyMod(shifted, generatorPoly);

    /* 系统码字: 消息 + 校验 */
    QVector<int> codeword;
    codeword.reserve(message.size() + genDeg);
    for (int b : message) codeword.append(b & 1);
    for (int i = 0; i < genDeg; ++i) {
        codeword.append((i < remainder.size()) ? (remainder[i] & 1) : 0);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(0);
    return codeword;
}

/**
 * @brief 计算BCH码伴随式
 *
 * 对接收码字r(x)在α, α^2, ..., α^{2t}处求值。
 * S_i = r(α^i)，若所有S_i = 0则无错误。
 * 使用GF(2^m)上的运算。
 *
 * @param received 接收码字比特序列
 * @param codeLength 码长n
 * @param errorCorrectionT 纠错能力t
 * @return 伴随式值列表[S_1, S_2, ..., S_{2t}]
 */
QVector<int> BchCode11::computeSyndromes(const QVector<int>& received,
                                          int codeLength, int errorCorrectionT)
{
    QVector<int> syndromes;
    if (received.isEmpty() || codeLength <= 0 || errorCorrectionT <= 0) return syndromes;

    int m = 1;
    while ((1 << m) - 1 < codeLength && m < 20) m++;
    int fieldSize = (1 << m) - 1;
    int primitivePoly = 0;
    /* 常用本原多项式 */
    if (m == 3) primitivePoly = 0xB;       /* x^3 + x + 1 */
    else if (m == 4) primitivePoly = 0x13; /* x^4 + x + 1 */
    else if (m == 5) primitivePoly = 0x25; /* x^5 + x^2 + 1 */
    else if (m == 6) primitivePoly = 0x43; /* x^6 + x + 1 */
    else if (m == 7) primitivePoly = 0x83; /* x^7 + x^3 + 1 */
    else if (m == 8) primitivePoly = 0x11D; /* x^8 + x^4 + x^3 + x^2 + 1 */
    else primitivePoly = (1 << m) | 0x3;   /* 回退: x^m + x + 1 */

    /* 构建GF(2^m)的对数/反对数表 */
    QVector<int> alphaTo(fieldSize + 1);
    QVector<int> logAlpha(fieldSize + 1, -1);
    alphaTo[0] = 1;
    logAlpha[1] = 0;
    for (int i = 1; i < fieldSize; ++i) {
        alphaTo[i] = alphaTo[i - 1] << 1;
        if (alphaTo[i] & (1 << m)) {
            alphaTo[i] ^= primitivePoly;
        }
        alphaTo[i] &= fieldSize;
        if (alphaTo[i] != 0) logAlpha[alphaTo[i]] = i;
    }

    /* 计算伴随式 S_i = r(α^i) */
    int numSyndromes = 2 * errorCorrectionT;
    syndromes.resize(numSyndromes);

    for (int i = 0; i < numSyndromes; ++i) {
        int sVal = 0;
        int alphaI = i + 1; /* α^{i+1} */
        for (int j = 0; j < received.size() && j < codeLength; ++j) {
            if (received[j] & 1) {
                /* sVal += α^{alphaI * j} */
                int exp = (alphaI * j) % fieldSize;
                if (exp == 0 && j > 0) exp = fieldSize;
                sVal ^= alphaTo[exp % fieldSize];
            }
        }
        syndromes[i] = sVal;
    }
    return syndromes;
}

/**
 * @brief BCH解码与纠错
 *
 * PGZ(Peterson-Gorenstein-Zierler)解码流程：
 * 1. 计算伴随式
 * 2. 若全零则无错，直接返回
 * 3. 构造伴随式矩阵，用PGZ算法求错误定位多项式
 * 4. Chien搜索找错误位置
 * 5. 翻转错误位完成纠错
 *
 * @param received 接收到的含误码字
 * @param codeLength 码长n
 * @param errorCorrectionT 纠错能力t
 * @return 纠错后的码字
 */
QVector<int> BchCode11::decode(const QVector<int>& received, int codeLength, int errorCorrectionT)
{
    QElapsedTimer timer;
    timer.start();

    if (received.isEmpty() || codeLength <= 0 || errorCorrectionT <= 0) {
        emit decodeCompleted(0);
        return received;
    }

    int correctedBits = 0;
    QVector<int> result = received;

    /* 步骤1: 计算伴随式 */
    QVector<int> syndromes = computeSyndromes(received, codeLength, errorCorrectionT);

    /* 检查是否有错误 */
    bool hasError = false;
    for (int s : syndromes) {
        if (s != 0) { hasError = true; break; }
    }

    if (hasError && syndromes.size() >= 2) {
        int t = errorCorrectionT;

        /* 步骤2: PGZ算法求错误定位多项式系数 */
        /* 构造伴随式矩阵 S[i][j] = s_{i+j} for i,j=0..ν-1 */
        int nu = qMin(t, syndromes.size() / 2);
        QVector<int> sigma = {1}; /* σ(x) = 1 */

        for (int v = nu; v >= 1; --v) {
            /* 尝试求解 ν×ν系统 */
            bool solvable = true;
            QVector<int> locCoeffs(v, 0);

            /* 高斯消元求解 σ_1, ..., σ_ν */
            QVector<QVector<int>> mat(v, QVector<int>(v + 1, 0));
            for (int i = 0; i < v; ++i) {
                for (int j = 0; j < v; ++j) {
                    int idx = i + j;
                    mat[i][j] = (idx < syndromes.size()) ? syndromes[idx] : 0;
                }
                int rhsIdx = i + v;
                mat[i][v] = (rhsIdx < syndromes.size()) ? syndromes[rhsIdx] : 0;
            }

            /* 简化前向消元 */
            for (int col = 0; col < v && solvable; ++col) {
                if (mat[col][col] == 0) {
                    solvable = false;
                    break;
                }
                for (int row = col + 1; row < v; ++row) {
                    if (mat[row][col] != 0) {
                        for (int k = col; k <= v; ++k) {
                            mat[row][k] ^= mat[col][k];
                        }
                    }
                }
            }

            if (solvable) {
                /* 回代 */
                for (int i = v - 1; i >= 0; --i) {
                    int val = mat[i][v];
                    for (int j = i + 1; j < v; ++j) {
                        val ^= mat[i][j] & locCoeffs[j];
                    }
                    locCoeffs[i] = val & 1;
                }

                sigma = {1};
                for (int c : locCoeffs) sigma.append(c);
                break;
            }
        }

        /* 步骤3: 穷举搜索错误位置 */
        /* σ(x)的根对应错误位置: 测试每个可能的α^{-i} */
        int m = 1;
        while ((1 << m) - 1 < codeLength && m < 20) m++;

        for (int pos = 0; pos < qMin(result.size(), codeLength); ++pos) {
            int eval = 0;
            int alphaPow = 1;
            for (int p = 0; p < pos; ++p) {
                alphaPow = (alphaPow << 1);
                if (alphaPow & (1 << m)) alphaPow ^= 0x3;
                alphaPow &= (1 << m) - 1;
            }

            /* 计算 σ(α^pos) */
            int alphaExp = 1;
            for (int s = 0; s < sigma.size(); ++s) {
                if (sigma[s] & 1) {
                    int pow = 1;
                    for (int p = 0; p < s * pos; ++p) {
                        pow = (pow << 1);
                        if (pow & (1 << m)) pow ^= 0x3;
                        pow &= (1 << m) - 1;
                    }
                    eval ^= pow;
                }
            }
            if (eval == 0 && pos < result.size()) {
                result[pos] ^= 1;
                correctedBits++;
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(correctedBits);
    return result;
}
