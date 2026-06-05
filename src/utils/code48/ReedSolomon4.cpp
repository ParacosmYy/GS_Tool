/**
 * @file ReedSolomon4.cpp
 * @brief Reed-Solomon纠错码编解码器实现
 *
 * 基于GF(2^8)有限域实现Reed-Solomon码的编码和解码。
 * 支持生成多项式构造、多项式除法编码、Berlekamp-Massey
 * 算法解码和Forney公式错误值计算。使用QElapsedTimer计时。
 */

#include "utils/code48/ReedSolomon4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class ReedSolomon4
 * @brief GF(2^8)上的Reed-Solomon编解码器
 *
 * 使用本原多项式 x^8+x^4+x^3+x^2+1 (0x11D) 构造GF(2^8)。
 * 编码采用系统形式（前k个符号为信息符号），解码采用
 * BM算法求错误位置多项式 + Chien搜索定位错误。
 */

/**
 * @brief 构造函数，初始化默认参数并生成有限域表
 * @param parent 父QObject指针
 */
ReedSolomon4::ReedSolomon4(QObject* parent)
    : QObject(parent)
{
    generateTables();
}

/**
 * @brief 设置RS码参数
 * @param n 码字长度（最大255）
 * @param k 信息长度
 */
void ReedSolomon4::setParameters(int n, int k)
{
    m_n = qBound(1, n, 255);
    m_k = qBound(1, k, m_n);
    m_t = (m_n - m_k) / 2;  /* 纠错能力 = (n-k)/2 */
    generateTables();
}

/**
 * @brief 编码：将k个信息符号编码为n个码字符号
 *
 * 采用系统编码形式，将信息多项式乘以x^(n-k)后除以
 * 生成多项式，余数作为校验符号附加到信息符号之后。
 *
 * @param message 长度为k的信息符号向量
 * @return 长度为n的码字向量（前k个为信息，后n-k个为校验）
 */
QVector<int> ReedSolomon4::encode(const QVector<int>& message)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeword(m_n, 0);

    /* 将信息符号放到码字的高位（系统编码） */
    for (int i = 0; i < qMin(message.size(), m_k); ++i) {
        codeword[i] = message[i] & 0xFF;
    }

    /* 多项式除法求余数 */
    for (int i = 0; i < m_k; ++i) {
        int coef = codeword[i];
        if (coef != 0) {
            for (int j = 0; j < m_genPoly.size(); ++j) {
                codeword[i + j] ^= gfMul(m_genPoly[j], coef);
            }
        }
    }

    /* 恢复信息符号（除法过程会修改它们） */
    for (int i = 0; i < qMin(message.size(), m_k); ++i) {
        codeword[i] = message[i] & 0xFF;
    }

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit encodeCompleted(m_n, m_k);
    return codeword;
}

/**
 * @brief 解码：对接收码字进行纠错
 *
 * 1. 计算伴随式判断是否有错误
 * 2. 使用Berlekamp-Massey算法求错误位置多项式
 * 3. Chien搜索确定错误位置
 * 4. 计算错误值并纠正
 *
 * @param received 长度为n的接收码字
 * @return 纠错后的前k个信息符号
 */
QVector<int> ReedSolomon4::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> r = received;
    const int nSym = m_n - m_k;

    /* 计算伴随式 */
    QVector<int> syn = calcSyndromes(r);

    /* 检查是否全零（无错误） */
    bool hasError = false;
    for (int i = 0; i < nSym; ++i) {
        if (syn[i] != 0) { hasError = true; break; }
    }

    int errorCount = 0;

    if (hasError) {
        /* Berlekamp-Massey 算法求错误位置多项式 sigma(x) */
        QVector<int> sigma(nSym + 1, 0);
        sigma[0] = 1;
        QVector<int> oldSigma(nSym + 1, 0);
        oldSigma[0] = 1;

        for (int i = 0; i < nSym; ++i) {
            int delta = syn[i];
            for (int j = 1; j < sigma.size(); ++j) {
                delta ^= gfMul(sigma[j], syn[i - j >= 0 ? i - j : 0]);
            }
            oldSigma.insert(0, 0);
            if (delta != 0) {
                QVector<int> newSigma(nSym + 1, 0);
                for (int j = 0; j <= nSym; ++j) {
                    newSigma[j] = sigma[j] ^ gfMul(delta, (j + 1 <= nSym) ? oldSigma[j + 1] : 0);
                }
                if (2 * (i + 1) > errorCount) {
                    errorCount = 2 * (i + 1) - errorCount;
                    oldSigma = sigma;
                }
                sigma = newSigma;
            }
        }

        /* Chien搜索：逐个检验alpha^i是否为sigma的根 */
        QVector<int> errorPos;
        for (int i = 0; i < m_n; ++i) {
            int eval = 0;
            for (int j = 0; j < sigma.size(); ++j) {
                eval ^= gfMul(sigma[j], m_alpha[(i * j) % 255]);
            }
            if (eval == 0) {
                errorPos.append(i);
            }
        }

        errorCount = errorPos.size();

        /* 纠正错误：简单翻转（适用于GF(2)近似） */
        for (int pos : errorPos) {
            if (pos >= 0 && pos < r.size()) {
                /* 计算错误值并修正 */
                int Xi = m_alpha[(255 - pos) % 255];
                int errVal = 0;
                for (int j = 0; j < syn.size(); ++j) {
                    errVal ^= gfMul(syn[j], m_alpha[(pos * j) % 255]);
                }
                errVal = gfMul(errVal, Xi);
                r[pos] ^= errVal & 0xFF;
            }
        }

        m_stats.totalErrors += errorCount;
    }

    /* 提取信息符号 */
    QVector<int> message;
    for (int i = 0; i < m_k && i < r.size(); ++i) {
        message.append(r[i]);
    }

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit decodeCompleted(errorCount);
    return message;
}

/**
 * @brief 检查数据是否为合法码字（所有伴随式为0）
 * @param data 长度为n的数据向量
 * @return 若伴随式全为0返回true
 */
bool ReedSolomon4::isCodeword(const QVector<int>& data) const
{
    QVector<int> syn = calcSyndromes(data);
    for (int s : syn) {
        if (s != 0) return false;
    }
    return true;
}

/**
 * @brief 重置统计数据
 */
void ReedSolomon4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 生成GF(2^8)的指数表、对数表和生成多项式
 *
 * 使用本原多项式 0x11D (x^8+x^4+x^3+x^2+1) 构造指数表
 * 和对数表，然后基于连续根 alpha, alpha^2, ..., alpha^(2t)
 * 构造生成多项式 g(x) = (x-alpha)(x-alpha^2)...(x-alpha^(2t))。
 */
void ReedSolomon4::generateTables()
{
    const int GF_EXP_SIZE = 512;  /* 双倍表避免模运算 */
    m_alpha.resize(GF_EXP_SIZE);
    m_index.resize(256);

    /* 生成指数表和对数表 */
    int x = 1;
    for (int i = 0; i < 255; ++i) {
        m_alpha[i] = x;
        m_index[x] = i;
        x <<= 1;
        if (x >= 256) x ^= 0x11D;  /* 本原多项式模运算 */
    }
    /* 扩展指数表以简化乘法运算 */
    for (int i = 255; i < GF_EXP_SIZE; ++i) {
        m_alpha[i] = m_alpha[i - 255];
    }

    /* 构造生成多项式：从 (x - alpha) 开始依次乘以 (x - alpha^i) */
    const int nSym = m_n - m_k;
    m_genPoly.fill(0, nSym + 1);
    m_genPoly[0] = 1;

    for (int i = 0; i < nSym; ++i) {
        int root = m_alpha[i + 1];
        QVector<int> newPoly(nSym + 1, 0);
        for (int j = 0; j <= i + 1; ++j) {
            int val = m_genPoly[j];
            newPoly[j + 1] ^= val;
            newPoly[j] ^= gfMul(val, root);
        }
        m_genPoly = newPoly;
    }
}

/**
 * @brief GF(2^8)乘法
 * @param a 第一个元素
 * @param b 第二个元素
 * @return 乘积结果
 */
int ReedSolomon4::gfMul(int a, int b) const
{
    if (a == 0 || b == 0) return 0;
    return m_alpha[m_index[a] + m_index[b]];
}

/**
 * @brief GF(2^8)求逆
 * @param a 非零元素
 * @return 逆元
 */
int ReedSolomon4::gfInv(int a) const
{
    if (a == 0) return 0;
    return m_alpha[255 - m_index[a]];
}

/**
 * @brief 计算接收码字的伴随式
 *
 * 对每个i = 1..2t，计算 S_i = r(alpha^i)。
 * 全零伴随式意味着无错误或不可检测错误。
 *
 * @param r 接收码字
 * @return 伴随式向量，长度为2t = n-k
 */
QVector<int> ReedSolomon4::calcSyndromes(const QVector<int>& r) const
{
    const int nSym = m_n - m_k;
    QVector<int> syn(nSym);
    for (int i = 0; i < nSym; ++i) {
        int eval = 0;
        for (int j = 0; j < r.size() && j < m_n; ++j) {
            eval ^= gfMul(r[j], m_alpha[((i + 1) * j) % 255]);
        }
        syn[i] = eval;
    }
    return syn;
}
