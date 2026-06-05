/**
 * @file ReedSolomon2.cpp
 * @brief Reed-Solomon编解码增强实现 — BM/Forney/擦除/Chien搜索
 */

#include "utils/code27/ReedSolomon2.h"

#include <QtMath>

ReedSolomon2::ReedSolomon2(int symbolSize, int nsym, int primitive, QObject* parent)
    : QObject(parent), m_symbolSize(symbolSize), m_nsym(nsym),
      m_primitive(primitive), m_fieldSize(1 << symbolSize)
{
    initGaloisField();
}

void ReedSolomon2::initGaloisField()
{
    m_gfExp.resize(2 * m_fieldSize);
    m_gfLog.resize(m_fieldSize);
    int x = 1;
    for (int i = 0; i < m_fieldSize - 1; ++i) {
        m_gfExp[i] = x;
        m_gfLog[x] = i;
        x <<= 1;
        if (x >= m_fieldSize) x ^= m_primitive;
        x &= (m_fieldSize - 1);
    }
    for (int i = m_fieldSize - 1; i < 2 * m_fieldSize; ++i)
        m_gfExp[i] = m_gfExp[i - (m_fieldSize - 1)];
    m_gfLog[0] = -1;
}

int ReedSolomon2::gfMul(int a, int b) const
{
    if (a == 0 || b == 0) return 0;
    return m_gfExp[m_gfLog[a] + m_gfLog[b]];
}

int ReedSolomon2::gfDiv(int a, int b) const
{
    if (a == 0) return 0;
    if (b == 0) return -1;
    int idx = m_gfLog[a] - m_gfLog[b];
    if (idx < 0) idx += m_fieldSize - 1;
    return m_gfExp[idx];
}

int ReedSolomon2::gfPow(int a, int n) const
{
    if (a == 0) return (n == 0) ? 1 : 0;
    return m_gfExp[(m_gfLog[a] * n) % (m_fieldSize - 1)];
}

int ReedSolomon2::gfInv(int a) const
{
    if (a == 0) return -1;
    return m_gfExp[m_fieldSize - 1 - m_gfLog[a]];
}

QVector<int> ReedSolomon2::encode(const QVector<int>& data)
{
    m_timing.start();
    ++m_stats.totalEncodes;
    QVector<int> result = data;
    /* 生成多项式: g(x) = (x - alpha^1)(x - alpha^2)...(x - alpha^nsym) */
    QVector<int> gen(m_nsym + 1, 0);
    gen[0] = 1;
    for (int i = 0; i < m_nsym; ++i) {
        QVector<int> newGen(m_nsym + 1, 0);
        for (int j = 0; j <= i; ++j) {
            newGen[j] ^= gfMul(gen[j], m_gfExp[i + 1]);
            newGen[j + 1] ^= gen[j];
        }
        gen = newGen;
    }
    /* 多项式除法取余 */
    QVector<int> remainder(m_nsym, 0);
    for (int i = 0; i < data.size(); ++i) {
        int coef = data[i] ^ remainder[0];
        remainder.erase(remainder.begin());
        remainder.append(0);
        if (coef != 0) {
            for (int j = 0; j < m_nsym; ++j)
                remainder[j] ^= gfMul(gen[j + 1], coef);
        }
    }
    result.append(remainder);
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;
    emit encoded(data.size(), m_nsym);
    return result;
}

QVector<int> ReedSolomon2::calcSyndromes(const QVector<int>& data) const
{
    QVector<int> synd(m_nsym);
    for (int i = 0; i < m_nsym; ++i) {
        int val = 0;
        int alphaI = m_gfExp[i + 1];
        int pw = 1;
        for (int j = 0; j < data.size(); ++j) {
            val ^= gfMul(data[j], pw);
            pw = gfMul(pw, alphaI);
        }
        synd[i] = val;
    }
    return synd;
}

QVector<int> ReedSolomon2::berlekampMassey(const QVector<int>& synd, int nsym) const
{
    QVector<int> errLoc = {1};
    QVector<int> oldLoc = {1};
    for (int i = 0; i < nsym; ++i) {
        int delta = synd[i];
        for (int j = 1; j < errLoc.size(); ++j)
            delta ^= gfMul(errLoc[j], synd[i - j]);
        oldLoc.insert(oldLoc.begin(), 0);
        if (delta != 0) {
            if (oldLoc.size() > errLoc.size()) {
                QVector<int> newLoc(oldLoc.size(), 0);
                for (int j = 0; j < oldLoc.size(); ++j)
                    newLoc[j] = gfMul(delta, oldLoc[j]);
                oldLoc = errLoc;
                for (int j = 0; j < errLoc.size(); ++j)
                    newLoc[j] ^= errLoc.size() > j ? errLoc[j] : 0;
                oldLoc = QVector<int>(errLoc.size(), 0);
                for (int j = 0; j < newLoc.size(); ++j) {
                    if (j < errLoc.size()) oldLoc[j] = errLoc[j];
                }
                errLoc = newLoc;
            } else {
                QVector<int> newLoc = errLoc;
                if (oldLoc.size() > newLoc.size())
                    newLoc.resize(oldLoc.size(), 0);
                for (int j = 0; j < oldLoc.size(); ++j)
                    newLoc[j] ^= gfMul(delta, oldLoc[j]);
                errLoc = newLoc;
            }
        }
    }
    return errLoc;
}

QVector<int> ReedSolomon2::chienSearch(const QVector<int>& errLoc, int n) const
{
    int numErr = errLoc.size() - 1;
    QVector<int> positions;
    for (int i = 0; i < n; ++i) {
        int eval = 0;
        for (int j = 0; j < errLoc.size(); ++j)
            eval ^= gfMul(errLoc[j], m_gfExp[(i * j) % (m_fieldSize - 1)]);
        if (eval == 0)
            positions.append(m_fieldSize - 1 - i);
    }
    if (positions.size() != numErr) return {};
    return positions;
}

QVector<int> ReedSolomon2::forneyAlgorithm(const QVector<int>& synd,
                                            const QVector<int>& errLoc,
                                            const QVector<int>& errPos) const
{
    /* 计算错误估值多项式 Omega(x) = Synd(x) * ErrLoc(x) mod x^nsym */
    int nsym = synd.size();
    QVector<int> omega(nsym, 0);
    for (int i = 0; i < nsym; ++i) {
        for (int j = 0; j < errLoc.size() && i + j < nsym; ++j)
            omega[i + j] ^= gfMul(synd[i], errLoc[j]);
    }
    /* 计算形式导数分母 */
    QVector<int> values(errPos.size(), 0);
    for (int i = 0; i < errPos.size(); ++i) {
        int xi = m_gfExp[errPos[i]];
        int xiInv = m_gfExp[m_fieldSize - 1 - errPos[i]];
        /* 错误位置多项式导数在xi的值 */
        int errLocDeriv = 0;
        for (int j = 1; j < errLoc.size(); j += 2)
            errLocDeriv ^= gfMul(errLoc[j], m_gfExp[((j - 1) * errPos[i]) % (m_fieldSize - 1)]);
        /* Omega(xiInv) */
        int omegaVal = 0;
        for (int j = 0; j < omega.size(); ++j)
            omegaVal ^= gfMul(omega[j], gfPow(xiInv, j));
        if (errLocDeriv != 0)
            values[i] = gfDiv(omegaVal, errLocDeriv);
    }
    return values;
}

QVector<int> ReedSolomon2::decode(const QVector<int>& received, const QVector<int>& erasePos)
{
    m_timing.start();
    ++m_stats.totalDecodes;

    /* 计算syndrome */
    QVector<int> synd = calcSyndromes(received);
    bool allZero = true;
    for (int s : synd) { if (s != 0) { allZero = false; break; } }
    if (allZero) {
        QVector<int> data(received.begin(), received.end() - m_nsym);
        emit decoded(0, true);
        return data;
    }

    /* 擦除定位多项式 */
    QVector<int> eraseLoc = {1};
    for (int pos : erasePos) {
        QVector<int> tmp = {1, m_gfExp[pos]};
        QVector<int> newErase(eraseLoc.size() + 1, 0);
        for (int i = 0; i < eraseLoc.size(); ++i)
            for (int j = 0; j < tmp.size(); ++j)
                newErase[i + j] ^= gfMul(eraseLoc[i], tmp[j]);
        eraseLoc = newErase;
    }

    /* Forney syndrome (擦除已知时简化) */
    QVector<int> forneySynd = synd;
    if (!erasePos.isEmpty()) {
        for (int pos : erasePos) {
            int xi = m_gfExp[pos];
            for (int i = 0; i < m_nsym - 1; ++i)
                forneySynd[i] = gfMul(forneySynd[i], xi) ^ forneySynd[i + 1];
        }
    }

    /* BM求错误定位多项式 */
    int numErasures = erasePos.size();
    QVector<int> errLoc = berlekampMassey(forneySynd, m_nsym - numErasures);

    /* 合并擦除和错误定位多项式 */
    QVector<int> fullLoc = eraseLoc;
    if (errLoc.size() > 1) {
        QVector<int> merged(fullLoc.size() + errLoc.size() - 1, 0);
        for (int i = 0; i < fullLoc.size(); ++i)
            for (int j = 0; j < errLoc.size(); ++j)
                merged[i + j] ^= gfMul(fullLoc[i], errLoc[j]);
        fullLoc = merged;
    }

    /* Chien搜索 */
    QVector<int> errPos = chienSearch(fullLoc, received.size());
    if (errPos.isEmpty() && fullLoc.size() > 1) {
        ++m_stats.totalUncorrectable;
        emit decoded(0, false);
        return {};
    }

    /* Forney公式求错误值 */
    QVector<int> errValues = forneyAlgorithm(synd, fullLoc, errPos);

    /* 纠错 */
    QVector<int> corrected = received;
    int totalErrors = errPos.size();
    for (int i = 0; i < errPos.size(); ++i) {
        int pos = (m_fieldSize - 1 - errPos[i]) % received.size();
        corrected[pos] ^= errValues[i];
    }
    m_stats.totalCorrectedErrors += totalErrors;
    m_timeSum += m_timing.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;
    emit decoded(totalErrors, true);
    corrected.erase(corrected.end() - m_nsym, corrected.end());
    return corrected;
}

void ReedSolomon2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
