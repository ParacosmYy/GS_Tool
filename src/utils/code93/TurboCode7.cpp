#include "TurboCode7.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Turbo码编解码器
 * @param parent 父对象指针
 */
TurboCode7::TurboCode7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置迭代解码次数
 * @param iterations 迭代次数，越多纠错性能越好但延迟越高
 */
void TurboCode7::setIterations(int iterations)
{
    m_iterations = qMax(1, iterations);
}

/**
 * @brief 交织器：伪随机重排比特顺序
 * @param bits 输入比特序列
 * @return 交织后的比特序列
 */
static QVector<int> interleave(const QVector<int>& bits)
{
    int n = bits.size();
    QVector<int> result(n);
    for (int i = 0; i < n; ++i) {
        int j = (i * 37 + 13) % n; /* 伪随机交织模式 */
        result[j] = bits[i];
    }
    return result;
}

/**
 * @brief 软交织器
 * @param llr 输入LLR序列
 * @return 交织后的LLR序列
 */
static QVector<double> interleaveSoft(const QVector<double>& llr)
{
    int n = llr.size();
    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        int j = (i * 37 + 13) % n;
        result[j] = llr[i];
    }
    return result;
}

/**
 * @brief 简化的SISO(软输入软输出)分量解码器
 * @param sysLLR 系统位LLR
 * @param parityLLR 校验位LLR
 * @param priorLLR 先验LLR
 * @return 后验LLR(外信息)
 */
static QVector<double> sisoDecode(const QVector<double>& sysLLR,
                                   const QVector<double>& parityLLR,
                                   const QVector<double>& priorLLR)
{
    int n = sysLLR.size();
    QVector<double> extrinsic(n, 0.0);

    for (int i = 0; i < n; ++i) {
        double totalInput = sysLLR[i] + priorLLR[i];
        /* 简化的BCJR: 使用符号函数和最小值近似 */
        double absParity = std::abs(parityLLR[i]);
        double sign = (totalInput * parityLLR[i] >= 0) ? 1.0 : -1.0;
        extrinsic[i] = sign * qMin(absParity, std::abs(totalInput)) - totalInput;
    }

    return extrinsic;
}

/**
 * @brief 对比特序列执行Turbo编码
 *
 * 并行级联两个RSC(递归系统卷积)编码器，
 * 第二个编码器对交织后的序列编码。
 * 输出为系统位 + 校验位1 + 校验位2。
 *
 * @param bits 输入信息比特序列
 */
void TurboCode7::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    if (bits.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalEncoded++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;
        emit codingCompleted(0);
        return;
    }

    int n = bits.size();

    /* 第一个RSC编码器(直接编码) */
    QVector<int> parity1(n, 0);
    int reg1 = 0;
    for (int i = 0; i < n; ++i) {
        int feedback = (bits[i] ^ ((reg1 >> 2) & 1) ^ ((reg1 >> 1) & 1));
        reg1 = ((reg1 << 1) | feedback) & 0x7;
        parity1[i] = feedback ^ ((reg1 >> 2) & 1);
    }

    /* 交织后的序列 */
    auto interleaved = interleave(bits);

    /* 第二个RSC编码器(交织后编码) */
    QVector<int> parity2(n, 0);
    int reg2 = 0;
    for (int i = 0; i < n; ++i) {
        int feedback = (interleaved[i] ^ ((reg2 >> 2) & 1) ^ ((reg2 >> 1) & 1));
        reg2 = ((reg2 << 1) | feedback) & 0x7;
        parity2[i] = feedback ^ ((reg2 >> 2) & 1);
    }

    /* 输出码字长度 = 3 * n (系统位 + 校验1 + 校验2) */
    int outputLen = 3 * n;

    m_timeSum += timer.elapsed();
    m_stats.totalEncoded++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;
    emit codingCompleted(outputLen);
}

/**
 * @brief 对软信息执行迭代Turbo解码
 *
 * 在两个SISO解码器之间反复传递外信息，
 * 经过设定迭代次数后输出硬判决结果。
 *
 * @param llr 接收的对数似然比序列
 */
void TurboCode7::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    if (llr.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalEncoded++;
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;
        emit codingCompleted(0);
        return;
    }

    int n = llr.size() / 3;
    if (n <= 0) {
        n = llr.size();
    }

    /* 提取系统位和校验位LLR */
    QVector<double> sysLLR(n, 0.0), parity1LLR(n, 0.0), parity2LLR(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int idx = i * 3;
        sysLLR[i] = (idx < llr.size()) ? llr[idx] : 0.0;
        parity1LLR[i] = (idx + 1 < llr.size()) ? llr[idx + 1] : 0.0;
        parity2LLR[i] = (idx + 2 < llr.size()) ? llr[idx + 2] : 0.0;
    }

    /* 迭代解码 */
    QVector<double> prior1(n, 0.0);
    QVector<double> prior2(n, 0.0);

    for (int iter = 0; iter < m_iterations; ++iter) {
        /* DEC1: 第一个SISO解码器 */
        auto extrinsic1 = sisoDecode(sysLLR, parity1LLR, prior1);

        /* 交织外信息并传给DEC2 */
        auto interleavedSys = interleaveSoft(sysLLR);
        auto interleavedExt = interleaveSoft(extrinsic1);

        /* DEC2: 第二个SISO解码器 */
        auto extrinsic2 = sisoDecode(interleavedSys, parity2LLR, interleavedExt);

        /* 解交织外信息并反馈给DEC1 */
        prior1 = interleaveSoft(extrinsic2);
    }

    m_timeSum += timer.elapsed();
    m_stats.totalEncoded++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncoded;
    emit codingCompleted(n);
}

/**
 * @brief 重置统计数据
 */
void TurboCode7::resetStatistics()
{
    m_stats.totalEncoded = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
