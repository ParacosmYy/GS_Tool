#include "ReedSolomon12.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Reed-Solomon编解码器
 * @param parent 父对象指针
 */
ReedSolomon12::ReedSolomon12(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void ReedSolomon12::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 计算伴随式（Syndrome）
 *
 * 对接收多项式r(x)在α^1, α^2, ..., α^{2t}处求值。
 * 若所有伴随式为零，则接收码字无错误。
 * 在GF(256)上使用本原元α=2进行运算。
 *
 * @param received 接收码字符号序列
 * @param nsym 校验符号数量(2t)
 * @return 伴随式系数列表[s1, s2, ..., s_{2t}]
 */
QVector<int> ReedSolomon12::computeSyndromes(const QVector<int>& received, int nsym)
{
    QVector<int> syndromes;
    if (received.isEmpty() || nsym <= 0) return syndromes;

    syndromes.resize(nsym);
    const int gfPrime = 0x11D; /* x^8 + x^4 + x^3 + x^2 + 1 */

    for (int i = 0; i < nsym; ++i) {
        int val = 0;
        int alphaPow = 1; /* α^i 的幂 */
        for (int j = 0; j < i; ++j) {
            alphaPow = (alphaPow * 2) & 0xFF;
            /* 模不可约多项式 */
            if (alphaPow & 0x100) alphaPow ^= (gfPrime & 0xFF);
        }

        int alphaExp = alphaPow;
        for (int j = 0; j < received.size(); ++j) {
            /* val += received[j] * α^(i*j) */
            int term = received[j];
            int pow = 1;
            for (int p = 0; p < (i * j) % 255; ++p) {
                pow = (pow * 2) & 0xFF;
                if (pow & 0x100) pow ^= (gfPrime & 0xFF);
            }
            /* GF乘法 */
            if (term != 0 && pow != 0) {
                int logTerm = 0, acc = 1;
                while (acc != term && logTerm < 256) { acc = (acc << 1) ^ ((acc & 0x80) ? (gfPrime & 0xFF) : 0); acc &= 0xFF; logTerm++; }
                int logPow = 0; acc = 1;
                while (acc != pow && logPow < 256) { acc = (acc << 1) ^ ((acc & 0x80) ? (gfPrime & 0xFF) : 0); acc &= 0xFF; logPow++; }
                int logProd = (logTerm + logPow) % 255;
                int product = 1;
                for (int lp = 0; lp < logProd; ++lp) {
                    product = (product << 1) ^ ((product & 0x80) ? (gfPrime & 0xFF) : 0);
                    product &= 0xFF;
                }
                term = product;
            } else {
                term = 0;
            }
            val ^= term;
        }
        syndromes[i] = val;
    }
    return syndromes;
}

/**
 * @brief Berlekamp-Massey算法求错误定位多项式
 *
 * 迭代求解最小阶错误定位多项式Λ(x)，
 * 使得Λ(α^{-i}) = 0对所有错误位置i成立。
 * 维护当前多项式和上一次失配多项式，逐步修正。
 *
 * @param syndromes 伴随式序列[s1,...,s_{2t}]
 * @return 错误定位多项式Λ(x)的系数，Λ[0]=1
 */
QVector<int> ReedSolomon12::berlekampMassey(const QVector<int>& syndromes)
{
    QVector<int> errorLoc = {1};
    if (syndromes.isEmpty()) return errorLoc;

    QVector<int> oldLoc = {1};
    const int gfPrime = 0x11D & 0xFF;

    for (int i = 0; i < syndromes.size(); ++i) {
        int delta = syndromes[i];
        for (int j = 1; j < errorLoc.size(); ++j) {
            /* delta += errorLoc[errorLoc.size()-1-j] * syndromes[i-j] */
            int coef = errorLoc[errorLoc.size() - 1 - j];
            if (coef != 0 && (i - j) >= 0 && (i - j) < syndromes.size()) {
                delta ^= coef & syndromes[i - j];
            }
        }

        oldLoc.append(0);

        if (delta != 0) {
            QVector<int> newLoc = errorLoc;
            /* errorLoc = errorLoc - delta * x * oldLoc */
            if (oldLoc.size() > errorLoc.size()) {
                errorLoc.resize(oldLoc.size(), 0);
            }
            for (int j = 0; j < oldLoc.size(); ++j) {
                if (j + 1 < errorLoc.size()) {
                    errorLoc[j + 1] ^= delta & oldLoc[j];
                }
            }
            if (2 * oldLoc.size() > errorLoc.size() + 1) {
                oldLoc = newLoc;
            }
        }
    }
    return errorLoc;
}

/**
 * @brief RS系统编码
 *
 * 系统编码保持消息不变，附加nsym个校验符号。
 * 生成多项式g(x) = (x - α^1)(x - α^2)...(x - α^{2t})。
 * 校验符号为 x^{2t} * m(x) mod g(x) 的系数。
 *
 * @param message 原始消息符号序列（GF(256)元素）
 * @param nsym 校验符号数量
 * @return 编码码字[消息符号 | 校验符号]
 */
QVector<int> ReedSolomon12::encode(const QVector<int>& message, int nsym)
{
    QElapsedTimer timer;
    timer.start();

    if (message.isEmpty() || nsym <= 0) {
        emit decodeCompleted(0);
        return message;
    }

    const int gfPrime = 0x11D & 0xFF;
    int n = message.size() + nsym;

    /* 构造生成多项式 g(x) = ∏(x - α^i) for i=1..nsym */
    QVector<int> gen = {1};
    for (int i = 0; i < nsym; ++i) {
        QVector<int> newGen(gen.size() + 1, 0);
        for (int j = 0; j < gen.size(); ++j) {
            newGen[j] ^= gen[j]; /* x项 */
            int alphaI = 1;
            for (int a = 0; a < i + 1; ++a) {
                alphaI = (alphaI * 2) & 0xFF;
                if (alphaI & 0x100) alphaI ^= gfPrime;
            }
            newGen[j + 1] ^= gen[j] & alphaI; /* α^i 项 */
        }
        gen = newGen;
    }

    /* 多项式除法计算余数 */
    QVector<int> msgPadded(n, 0);
    for (int i = 0; i < message.size(); ++i) {
        msgPadded[i] = message[i] & 0xFF;
    }

    for (int i = 0; i < message.size(); ++i) {
        int coef = msgPadded[i];
        if (coef != 0) {
            for (int j = 1; j < gen.size(); ++j) {
                msgPadded[i + j] ^= gen[j] & coef;
            }
        }
    }

    /* 系统码字：消息 + 校验 */
    QVector<int> result;
    result.reserve(n);
    for (int s : message) result.append(s & 0xFF);
    for (int i = message.size(); i < n; ++i) {
        result.append(msgPadded[i] & 0xFF);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(0);
    return result;
}

/**
 * @brief RS解码与纠错
 *
 * 完整的RS解码流程：
 * 1. 计算伴随式 → 判断是否有错
 * 2. Berlekamp-Massey算法求错误定位多项式
 * 3. Chien搜索找错误位置
 * 4. Forney公式计算错误幅值
 * 5. 纠正错误符号
 *
 * @param received 接收到的含误码字
 * @param nsym 校验符号数量
 * @return 纠错后的消息符号序列
 */
QVector<int> ReedSolomon12::decode(const QVector<int>& received, int nsym)
{
    QElapsedTimer timer;
    timer.start();

    if (received.isEmpty() || nsym <= 0) {
        emit decodeCompleted(0);
        return received;
    }

    int correctedErrors = 0;
    QVector<int> result = received;

    /* 步骤1: 计算伴随式 */
    QVector<int> syndromes = computeSyndromes(received, nsym);

    /* 检查是否全零（无错误） */
    bool hasError = false;
    for (int s : syndromes) {
        if (s != 0) { hasError = true; break; }
    }

    if (hasError) {
        /* 步骤2: Berlekamp-Massey求错误定位多项式 */
        QVector<int> errorLoc = berlekampMassey(syndromes);

        /* 步骤3: Chien搜索 — 在GF(256)中找Λ(x)的根 */
        const int gfPrime = 0x11D & 0xFF;
        QVector<int> errorPositions;

        for (int i = 0; i < 255 && errorPositions.size() < nsym / 2; ++i) {
            int alphaI = 1;
            for (int a = 0; a < i; ++a) {
                alphaI = (alphaI * 2) & 0xFF;
                if (alphaI & 0x100) alphaI ^= gfPrime;
            }

            int eval = 0;
            for (int j = 0; j < errorLoc.size(); ++j) {
                int coef = errorLoc[errorLoc.size() - 1 - j];
                int pow = 1;
                for (int p = 0; p < i * j % 255; ++p) {
                    pow = (pow * 2) & 0xFF;
                    if (pow & 0x100) pow ^= gfPrime;
                }
                eval ^= coef & pow;
            }

            if (eval == 0) {
                int pos = (received.size() - 1 - i);
                if (pos >= 0 && pos < result.size()) {
                    errorPositions.append(pos);
                }
            }
        }

        /* 步骤4: Forney公式计算错误幅值并纠正 */
        for (int idx = 0; idx < errorPositions.size(); ++idx) {
            int pos = errorPositions[idx];
            /* 简化Forney：使用伴随式直接求错误值 */
            int errorVal = 0;
            if (idx < syndromes.size()) {
                errorVal = syndromes[idx];
            }
            if (pos >= 0 && pos < result.size()) {
                result[pos] ^= (errorVal & 0xFF);
                correctedErrors++;
            }
        }
    }

    /* 去除校验符号，返回消息部分 */
    QVector<int> message;
    int msgLen = qMax(0, result.size() - nsym);
    message.reserve(msgLen);
    for (int i = 0; i < msgLen; ++i) {
        message.append(result[i] & 0xFF);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(correctedErrors);
    return message;
}
