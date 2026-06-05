/**
 * @file ReedSolomon2.cpp
 * @brief Reed-Solomon纠错码 GF(2^8) 实现
 */

#include "utils/reed_solomon2/ReedSolomon2.h"

#include <QElapsedTimer>
#include <algorithm>

ReedSolomon2::ReedSolomon2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
    , m_nsym(10)
    , m_tablesInit(false)
{
    initTables();
}

void ReedSolomon2::initTables()
{
    if (m_tablesInit) return;

    /* GF(2^8) 本原多项式: x^8 + x^4 + x^3 + x^2 + 1 = 0x11D */
    quint32 prim = 0x11D;
    quint32 x = 1;
    for (int i = 0; i < 255; ++i) {
        m_gfExp[i] = static_cast<quint8>(x);
        m_gfLog[x] = static_cast<quint8>(i);
        x <<= 1;
        if (x & 0x100) x ^= prim;
    }
    /* 扩展指数表方便计算 */
    for (int i = 255; i < 512; ++i) {
        m_gfExp[i] = m_gfExp[i - 255];
    }
    m_gfLog[0] = 0;
    m_tablesInit = true;
}

quint8 ReedSolomon2::gfMul(quint8 a, quint8 b) const
{
    if (a == 0 || b == 0) return 0;
    return m_gfExp[m_gfLog[a] + m_gfLog[b]];
}

quint8 ReedSolomon2::gfDiv(quint8 a, quint8 b) const
{
    if (a == 0) return 0;
    if (b == 0) return 0; /* 除零保护 */
    int logA = m_gfLog[a];
    int logB = m_gfLog[b];
    int diff = logA - logB;
    if (diff < 0) diff += 255;
    return m_gfExp[diff];
}

quint8 ReedSolomon2::gfInv(quint8 a) const
{
    if (a == 0) return 0;
    return m_gfExp[255 - m_gfLog[a]];
}

quint8 ReedSolomon2::gfPolyEval(const QVector<quint8>& poly, quint8 x) const
{
    /* Horner法则求值 */
    quint8 result = 0;
    for (int i = 0; i < poly.size(); ++i) {
        result = gfMul(result, x) ^ poly[i];
    }
    return result;
}

QVector<quint8> ReedSolomon2::gfPolyMul(const QVector<quint8>& a,
                                          const QVector<quint8>& b) const
{
    int lenA = a.size();
    int lenB = b.size();
    QVector<quint8> result(lenA + lenB - 1, 0);
    for (int i = 0; i < lenA; ++i) {
        for (int j = 0; j < lenB; ++j) {
            result[i + j] ^= gfMul(a[i], b[j]);
        }
    }
    return result;
}

void ReedSolomon2::setParameters(int nsym)
{
    m_nsym = qMax(1, nsym);
}

QVector<quint8> ReedSolomon2::calcSyndromes(const QByteArray& data) const
{
    QVector<quint8> synd(m_nsym + 1, 0);
    synd[0] = 0;
    for (int i = 0; i < m_nsym; ++i) {
        quint8 val = 0;
        for (int j = 0; j < data.size(); ++j) {
            val = static_cast<quint8>(data[j]) ^
                  gfMul(val, m_gfExp[i + 1]);
        }
        synd[i + 1] = val;
    }
    return synd;
}

QVector<quint8> ReedSolomon2::berlekampMassey(
    const QVector<quint8>& synd) const
{
    QVector<quint8> errLoc = {1};
    QVector<quint8> oldLoc = {1};

    for (int i = 0; i < m_nsym; ++i) {
        /* 计算差异 */
        quint8 delta = synd[i + 1];
        for (int j = 1; j < errLoc.size(); ++j) {
            delta ^= gfMul(errLoc[errLoc.size() - 1 - j], synd[i + 1 - j]);
        }

        QVector<quint8> newLoc = errLoc;
        if (delta != 0) {
            QVector<quint8> shifted(oldLoc.size() + 1, 0);
            for (int j = 0; j < oldLoc.size(); ++j) {
                shifted[j] = gfMul(delta, oldLoc[j]);
            }
            /* 对齐长度 */
            while (newLoc.size() < shifted.size()) {
                newLoc.insert(newLoc.begin(), 0);
            }
            while (shifted.size() < newLoc.size()) {
                shifted.insert(shifted.begin(), 0);
            }
            for (int j = 0; j < newLoc.size(); ++j) {
                newLoc[j] ^= shifted[j];
            }
        }

        if (2 * oldLoc.size() <= i) {
            oldLoc = errLoc;
        }
        errLoc = newLoc;
    }

    return errLoc;
}

QVector<int> ReedSolomon2::chienSearch(const QVector<quint8>& errLoc,
                                         int dataLen) const
{
    QVector<int> positions;
    int numErr = errLoc.size() - 1;

    for (int i = 0; i < dataLen; ++i) {
        quint8 eval = gfPolyEval(errLoc, m_gfExp[dataLen - 1 - i]);
        if (eval == 0) {
            positions.append(i);
        }
        if (positions.size() >= numErr) break;
    }

    return positions;
}

QVector<quint8> ReedSolomon2::forneyAlgorithm(
    const QVector<quint8>& synd,
    const QVector<quint8>& errLoc,
    const QVector<int>& errPos) const
{
    QVector<quint8> errVals(errPos.size(), 0);
    QVector<quint8> errLocRev = errLoc;
    std::reverse(errLocRev.begin(), errLocRev.end());

    for (int idx = 0; idx < errPos.size(); ++idx) {
        int pos = errPos[idx];
        quint8 xi = m_gfExp[pos];
        quint8 xiInv = gfInv(xi);

        /* 伴随式求值 */
        quint8 omega = 0;
        for (int i = 0; i < m_nsym; ++i) {
            omega ^= gfMul(synd[m_nsym - i], gfPow(xiInv, i));
        }

        /* 形式导数求值 */
        quint8 errLocDeriv = 0;
        for (int i = 0; i < errLocRev.size(); i += 2) {
            errLocDeriv ^= errLocRev[i];
        }

        if (errLocDeriv == 0) continue;
        errVals[idx] = gfDiv(omega, errLocDeriv);
    }

    return errVals;
}

quint8 ReedSolomon2::gfPow(quint8 base, int exp) const
{
    if (exp == 0) return 1;
    if (base == 0) return 0;
    return m_gfExp[(static_cast<int>(m_gfLog[base]) * exp) % 255];
}

QByteArray ReedSolomon2::encode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    /* 生成多项式: (x - alpha^1)(x - alpha^2)...(x - alpha^nsym) */
    QVector<quint8> gen = {1};
    for (int i = 0; i < m_nsym; ++i) {
        QVector<quint8> term = {1, m_gfExp[i + 1]};
        gen = gfPolyMul(gen, term);
    }

    /* 系统编码: 附加nsym个零, 然后计算余数 */
    QVector<quint8> msg(data.size() + m_nsym, 0);
    for (int i = 0; i < data.size(); ++i) {
        msg[i] = static_cast<quint8>(data[i]);
    }

    /* 多项式长除法计算校验符号 */
    for (int i = 0; i < data.size(); ++i) {
        quint8 coef = msg[i];
        if (coef != 0) {
            for (int j = 1; j < gen.size(); ++j) {
                msg[i + j] ^= gfMul(gen[j], coef);
            }
        }
    }

    QByteArray result = data;
    for (int i = 0; i < m_nsym; ++i) {
        result.append(static_cast<char>(msg[data.size() + i]));
    }

    m_stats.totalEncoded++;
    m_timeSum += timer.elapsed();
    double total = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

QByteArray ReedSolomon2::decode(const QByteArray& received)
{
    QElapsedTimer timer;
    timer.start();

    int dataLen = received.size() - m_nsym;
    bool success = true;
    int corrected = 0;

    if (dataLen <= 0) {
        emit decodingCompleted(false, 0);
        return received;
    }

    /* 计算伴随式 */
    QVector<quint8> synd = calcSyndromes(received);

    /* 检查是否有错误 */
    bool hasError = false;
    for (int i = 1; i <= m_nsym; ++i) {
        if (synd[i] != 0) { hasError = true; break; }
    }

    QByteArray result = received;

    if (hasError) {
        /* Berlekamp-Massey */
        QVector<quint8> errLoc = berlekampMassey(synd);

        /* Chien搜索 */
        QVector<int> errPos = chienSearch(errLoc, received.size());

        if (errPos.isEmpty() || errPos.size() > m_nsym / 2) {
            success = false;
        } else {
            /* Forney算法 */
            QVector<quint8> errVals = forneyAlgorithm(synd, errLoc, errPos);

            /* 纠正错误 */
            for (int i = 0; i < errPos.size() && i < errVals.size(); ++i) {
                int pos = errPos[i];
                if (pos >= 0 && pos < result.size()) {
                    result[pos] = static_cast<char>(
                        result[pos] ^ errVals[i]);
                    corrected++;
                }
            }
        }
    }

    /* 截断校验符号 */
    QByteArray decoded = result.left(dataLen);

    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    double total = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit decodingCompleted(success, corrected);
    return decoded;
}

void ReedSolomon2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
