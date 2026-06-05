/**
 * @file ConvolutionalInterleaver.cpp
 * @brief 卷积交织器实现 — FIFO移位寄存器/突发错误分散
 */

#include "utils/code22/ConvolutionalInterleaver.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
ConvolutionalInterleaver::ConvolutionalInterleaver(QObject* parent)
    : QObject(parent)
    , m_branches(4)
    , m_delayPerBranch(2)
    , m_currentBranch(0)
    , m_currentBranchDe(0)
{
    configure(m_branches, m_delayPerBranch);
}

/** @brief 配置交织参数 @param branches 分支数 @param delayPerBranch 每分支延迟 */
void ConvolutionalInterleaver::configure(int branches, int delayPerBranch)
{
    m_branches = qMax(2, branches);
    m_delayPerBranch = qMax(1, delayPerBranch);
    m_currentBranch = 0;
    m_currentBranchDe = 0;

    /* 交织: 第i条分支延迟 i*M 个符号 */
    m_fifo.resize(m_branches);
    for (int i = 0; i < m_branches; ++i) {
        int delay = i * m_delayPerBranch;
        m_fifo[i].resize(delay, 0);
    }

    /* 解交织: 第i条分支延迟 (B-1-i)*M 个符号 */
    m_fifoDe.resize(m_branches);
    for (int i = 0; i < m_branches; ++i) {
        int delay = (m_branches - 1 - i) * m_delayPerBranch;
        m_fifoDe[i].resize(delay, 0);
    }
}

/** @brief 交织单个符号 @param symbol 输入符号 @return 交织后符号 */
quint8 ConvolutionalInterleaver::interleave(quint8 symbol)
{
    quint8 out = shiftRegister(m_currentBranch, symbol, false);
    m_currentBranch = (m_currentBranch + 1) % m_branches;
    return out;
}

/** @brief 解交织单个符号 @param symbol 输入符号 @return 解交织后符号 */
quint8 ConvolutionalInterleaver::deinterleave(quint8 symbol)
{
    quint8 out = shiftRegister(m_currentBranchDe, symbol, true);
    m_currentBranchDe = (m_currentBranchDe + 1) % m_branches;
    return out;
}

/** @brief 批量交织 @param data 输入数据 @return 交织后数据 */
QByteArray ConvolutionalInterleaver::interleaveBlock(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    result.reserve(data.size() + totalDelay());

    /* 先填充延迟补偿(全零前缀) */
    int delay = totalDelay();
    for (int i = 0; i < delay; ++i) {
        result.append(static_cast<char>(interleave(0)));
    }

    /* 交织有效数据 */
    for (int i = 0; i < data.size(); ++i) {
        result.append(static_cast<char>(interleave(
            static_cast<quint8>(data[i]))));
    }

    /* 统计突发错误分散能力 */
    int burstLen = 0;
    int maxBurst = 0;
    for (int i = 0; i < data.size(); ++i) {
        if (static_cast<quint8>(data[i]) != 0) {
            burstLen++;
            maxBurst = qMax(maxBurst, burstLen);
        } else {
            burstLen = 0;
        }
    }

    m_stats.totalInterleaves++;
    m_stats.totalSymbolsProcessed += data.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInterleaves
                              + m_stats.totalDeinterleaves);
    m_stats.totalBurstErrorsSpread += maxBurst;

    emit interleaveComplete(data.size());
    return result;
}

/** @brief 批量解交织 @param data 输入数据 @return 解交织后数据 */
QByteArray ConvolutionalInterleaver::deinterleaveBlock(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    result.reserve(data.size());

    /* 解交织所有数据(包括延迟前缀) */
    for (int i = 0; i < data.size(); ++i) {
        result.append(static_cast<char>(deinterleave(
            static_cast<quint8>(data[i]))));
    }

    /* 去除延迟补偿部分 */
    int delay = totalDelay();
    if (result.size() > delay) {
        result = result.mid(delay);
    }

    m_stats.totalDeinterleaves++;
    m_stats.totalSymbolsProcessed += data.size();
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalInterleaves
                              + m_stats.totalDeinterleaves);

    emit deinterleaveComplete(result.size());
    return result;
}

/** @brief 获取总延迟 @return 延迟符号数 */
int ConvolutionalInterleaver::totalDelay() const
{
    /* 总延迟 = B*(B-1)*M / 2 */
    return m_branches * (m_branches - 1) * m_delayPerBranch / 2;
}

/** @brief 获取分支数 @return 分支数 */
int ConvolutionalInterleaver::branchCount() const
{
    return m_branches;
}

/** @brief 重置FIFO状态 */
void ConvolutionalInterleaver::resetState()
{
    m_currentBranch = 0;
    m_currentBranchDe = 0;
    for (int i = 0; i < m_branches; ++i) {
        m_fifo[i].fill(0);
        m_fifoDe[i].fill(0);
    }
}

/** @brief 重置统计 */
void ConvolutionalInterleaver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief FIFO移位寄存器操作 @param branch 分支号 @param symbol 输入符号 @param inverse 是否解交织 @return 输出符号 */
quint8 ConvolutionalInterleaver::shiftRegister(int branch, quint8 symbol,
                                                bool inverse)
{
    auto& fifo = inverse ? m_fifoDe : m_fifo;
    if (branch < 0 || branch >= fifo.size()) return symbol;

    auto& reg = fifo[branch];
    if (reg.isEmpty()) return symbol;

    /* 弹出最老的符号，推入新符号 */
    quint8 out = reg.first();
    reg.removeFirst();
    reg.append(symbol);
    return out;
}

/**
 * @brief 验证交织/解交织往返一致性
 *
 * 对一组测试数据执行交织后解交织，检查是否恢复原始数据。
 * 用于验证交织器配置和FIFO状态正确性。
 *
 * @param testData 测试数据
 * @return true=往返一致
 */
bool ConvolutionalInterleaver::verifyRoundTrip(const QByteArray& testData)
{
    if (testData.isEmpty()) return true;

    /* 保存当前状态 */
    auto savedFifo = m_fifo;
    auto savedFifoDe = m_fifoDe;
    int savedBranch = m_currentBranch;
    int savedBranchDe = m_currentBranchDe;

    /* 执行交织 */
    QByteArray interleaved;
    interleaved.reserve(testData.size());
    for (int i = 0; i < testData.size(); ++i) {
        interleaved.append(static_cast<char>(
            interleave(static_cast<quint8>(testData[i]))));
    }

    /* 执行解交织 */
    QByteArray recovered;
    recovered.reserve(interleaved.size());
    for (int i = 0; i < interleaved.size(); ++i) {
        recovered.append(static_cast<char>(
            deinterleave(static_cast<quint8>(interleaved[i]))));
    }

    /* 恢复状态 */
    m_fifo = savedFifo;
    m_fifoDe = savedFifoDe;
    m_currentBranch = savedBranch;
    m_currentBranchDe = savedBranchDe;

    /* 比较(考虑延迟偏移) */
    int delay = totalDelay();
    if (recovered.size() <= delay) return false;

    for (int i = 0; i < testData.size() && (i + delay) < recovered.size(); ++i) {
        if (testData[i] != recovered[i + delay]) return false;
    }

    return true;
}
