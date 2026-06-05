/**
 * @file PolarCode.cpp
 * @brief Polar码编解码器实现
 */

#include "PolarCode.h"

#include <QElapsedTimer>
#include <cmath>
#include <algorithm>
#include <QtGlobal>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

PolarCode::PolarCode(int n, int k, QObject* parent)
    : QObject(parent)
    , m_n(n)
    , m_N(1 << n)
    , m_K(k)
{
    m_numStates = 1; // 不需要trellis状态
    computeReliability();
    selectFrozenBits();
}

PolarCode::~PolarCode() = default;

// ═══════════════════════════════════════════════════════════
// 编码
// ═══════════════════════════════════════════════════════════

QVector<quint8> PolarCode::encode(const QVector<quint8>& infoBits)
{
    if (infoBits.size() != m_K) {
        emit error(tr("Polar码编码错误: 信息位长度%1不等于K=%2")
                   .arg(infoBits.size()).arg(m_K));
        return {};
    }

    // 构造u向量: 信息位插入非冻结位, 冻结位置0
    QVector<quint8> u(m_N, 0);
    int infoIdx = 0;
    for (int i = 0; i < m_N && infoIdx < m_K; ++i) {
        if (!m_frozenSet.contains(i)) {
            u[i] = infoBits[infoIdx++];
        }
    }

    // Polar变换: x = u * G_N (递归XOR)
    polarTransform(u);

    m_stats.totalEncodes++;
    m_stats.totalBits += static_cast<quint64>(m_N);
    emit encoded(m_N);
    return u;
}

// ═══════════════════════════════════════════════════════════
// 解码
// ═══════════════════════════════════════════════════════════

QVector<quint8> PolarCode::decodeSC(const QVector<quint8>& receivedBits)
{
    if (receivedBits.size() != m_N) {
        emit error(tr("Polar码SC解码错误: 接收长度%1不等于N=%2")
                   .arg(receivedBits.size()).arg(m_N));
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    // 将硬判决转换为LLR
    QVector<double> llr(m_N);
    for (int i = 0; i < m_N; ++i) {
        llr[i] = (receivedBits[i] == 0) ? 10.0 : -10.0;
    }

    QVector<quint8> decoded(m_N, 0);
    scDecodeRecursive(llr, decoded, 0, m_N);

    // 提取信息位
    QVector<quint8> infoBits;
    infoBits.reserve(m_K);
    for (int i = 0; i < m_N; ++i) {
        if (!m_frozenSet.contains(i)) {
            infoBits.append(decoded[i]);
        }
    }

    m_stats.totalDecodes++;
    m_stats.totalBits += static_cast<quint64>(m_N);
    m_stats.avgDecodeTimeMs = (m_stats.avgDecodeTimeMs * (m_stats.totalDecodes - 1) +
                               timer.elapsed()) / static_cast<double>(m_stats.totalDecodes);

    emit decoded(m_K);
    return infoBits;
}

QVector<quint8> PolarCode::decodeSCLlr(const QVector<double>& llr)
{
    if (llr.size() != m_N) {
        emit error(tr("Polar码LLR解码错误: LLR长度不匹配"));
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    QVector<quint8> decoded(m_N, 0);
    scDecodeRecursive(llr, decoded, 0, m_N);

    QVector<quint8> infoBits;
    infoBits.reserve(m_K);
    for (int i = 0; i < m_N; ++i) {
        if (!m_frozenSet.contains(i)) {
            infoBits.append(decoded[i]);
        }
    }

    m_stats.totalDecodes++;
    m_stats.totalBits += static_cast<quint64>(m_N);
    m_stats.avgDecodeTimeMs = (m_stats.avgDecodeTimeMs * (m_stats.totalDecodes - 1) +
                               timer.elapsed()) / static_cast<double>(m_stats.totalDecodes);

    emit decoded(m_K);
    return infoBits;
}

// ═══════════════════════════════════════════════════════════
// 配置
// ═══════════════════════════════════════════════════════════

int PolarCode::blockLength() const { return m_N; }
int PolarCode::infoLength() const { return m_K; }
double PolarCode::codeRate() const { return static_cast<double>(m_K) / static_cast<double>(m_N); }
QVector<int> PolarCode::frozenPositions() const { return m_frozenPositions; }

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

PolarCode::Stats PolarCode::stats() const { return m_stats; }
void PolarCode::resetStatistics() { m_stats = Stats{}; }

// ═══════════════════════════════════════════════════════════
// 内部: 可靠性计算(巴塔恰里亚参数)
// ═══════════════════════════════════════════════════════════

void PolarCode::computeReliability()
{
    // 使用简化巴塔恰里亚参数递归计算
    m_reliability.resize(m_N);
    m_reliability[0] = 0.5; // 初始值

    for (int stage = 0; stage < m_n; ++stage) {
        const int blockSize = 1 << stage;
        QVector<double> newReliability(2 * blockSize);
        for (int i = 0; i < blockSize; ++i) {
            // W^-: 可靠性降低
            newReliability[2 * i] = m_reliability[i] * m_reliability[i];
            // W^+: 可靠性提高
            newReliability[2 * i + 1] = 2.0 * m_reliability[i] -
                                          m_reliability[i] * m_reliability[i];
        }
        m_reliability = newReliability;
    }
}

void PolarCode::selectFrozenBits()
{
    // 按可靠性排序, 选择最不可靠的N-K位作为冻结位
    QVector<QPair<double, int>> indexed;
    indexed.reserve(m_N);
    for (int i = 0; i < m_N; ++i) {
        indexed.append(qMakePair(m_reliability[i], i));
    }

    std::sort(indexed.begin(), indexed.end(),
              [](const QPair<double, int>& a, const QPair<double, int>& b) {
                  return a.first < b.first;
              });

    m_frozenPositions.clear();
    m_frozenSet.clear();

    const int frozenCount = m_N - m_K;
    for (int i = 0; i < frozenCount && i < indexed.size(); ++i) {
        m_frozenPositions.append(indexed[i].second);
        m_frozenSet.insert(indexed[i].second);
    }
}

// ═══════════════════════════════════════════════════════════
// 内部: Polar变换
// ═══════════════════════════════════════════════════════════

void PolarCode::polarTransform(QVector<quint8>& bits) const
{
    for (int stage = 0; stage < m_n; ++stage) {
        const int step = 1 << (stage + 1);
        for (int i = 0; i < m_N; i += step) {
            for (int j = 0; j < (step / 2); ++j) {
                bits[i + j] = bits[i + j] ^ bits[i + j + step / 2];
            }
        }
    }
}

// ═══════════════════════════════════════════════════════════
// 内部: SC解码递归
// ═══════════════════════════════════════════════════════════

void PolarCode::scDecodeRecursive(const QVector<double>& llr,
                                   QVector<quint8>& decoded,
                                   int offset, int length)
{
    if (length == 1) {
        // 叶节点: 判决
        if (m_frozenSet.contains(offset)) {
            decoded[offset] = 0; // 冻结位固定为0
        } else {
            decoded[offset] = (llr[offset] >= 0) ? 0 : 1;
        }
        return;
    }

    const int half = length / 2;

    // 计算左子节点LLR (f函数)
    QVector<double> leftLlr(half);
    for (int i = 0; i < half; ++i) {
        leftLlr[i] = llrLeft(llr[i], llr[i + half]);
    }

    // 递归左子树
    scDecodeRecursive(leftLlr, decoded, offset, half);

    // 计算右子节点LLR (g函数)
    QVector<double> rightLlr(half);
    for (int i = 0; i < half; ++i) {
        rightLlr[i] = llrRight(llr[i], llr[i + half], decoded[offset + i]);
    }

    // 递归右子树
    scDecodeRecursive(rightLlr, decoded, offset + half, half);
}

double PolarCode::llrLeft(double a, double b) const
{
    // f(a, b) = sign(a)*sign(b)*min(|a|, |b|)
    const double absA = std::abs(a);
    const double absB = std::abs(b);
    const double signA = (a >= 0) ? 1.0 : -1.0;
    const double signB = (b >= 0) ? 1.0 : -1.0;
    return signA * signB * std::min(absA, absB);
}

double PolarCode::llrRight(double a, double b, quint8 u) const
{
    // g(a, b, u) = b + (1 - 2*u) * a
    return b + (1.0 - 2.0 * static_cast<double>(u)) * a;
}
