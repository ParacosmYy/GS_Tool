/**
 * @file TurboCode3.cpp
 * @brief 双二进制Turbo码实现 - MAP解码迭代
 *
 * 双二进制Turbo码将两个比特组成一个符号(00/01/10/11)，
 * 使用MAP(Maximum A Posteriori)算法进行迭代解码，
 * 每次迭代在两个分量解码器之间交换外信息。
 */

#include "utils/code36/TurboCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数，初始化默认参数(块长6144, 8次迭代)
 * @param parent 父QObject
 */
TurboCode3::TurboCode3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 配置Turbo码参数
 * @param blockLen 信息块长度(符号数)
 * @param numIter 迭代次数
 */
void TurboCode3::configure(int blockLen, int numIter)
{
    m_blockLen = qMax(16, blockLen);
    m_numIter = qMax(1, numIter);
}

/**
 * @brief 设置是否启用早停(CRC校验通过即停止)
 * @param enable true启用早停
 */
void TurboCode3::setEarlyTermination(bool enable)
{
    m_earlyStop = enable;
}

/**
 * @brief Turbo编码器
 *
 * 双二进制编码:
 * 1. 将输入比特两两组成符号
 * 2. 通过两个递归系统卷积(RSC)编码器(第二个使用交织后的数据)
 * 3. 输出: 系统位 + 校验位1 + 校验位2
 *
 * @param bits 输入比特序列(长度应为blockLen的整数倍)
 * @return 编码后的比特序列
 */
QVector<int> TurboCode3::encode(const QVector<int>& bits) const
{
    const int n = bits.size();
    const int numSymbols = n / 2;
    QVector<int> encoded;

    if (n < 2) return encoded;

    encoded.reserve(n * 2);

    /* 交织器: 使用基于素数的伪随机交织 */
    auto interleave = [](int idx, int total) -> int {
        /* 简化的素数模交织 */
        int p = 37;
        while (p < total / 2) p = p * 2 + 1;
        return (idx * p + 1) % total;
    };

    /* RSC编码器生成多项式: [1, 1+D+D^2+D^3] (反馈+前馈) */
    auto rscEncode = [](const QVector<int>& sysBits, const QVector<int>& parityBits,
                        bool interleaved) -> QVector<int> {
        Q_UNUSED(interleaved);
        int len = sysBits.size();
        QVector<int> output;
        output.reserve(len);

        int state = 0; /* 3位寄存器状态 */

        for (int i = 0; i < len; ++i) {
            int u = sysBits[i];

            /* 反馈 = u XOR state_bit2 */
            int fb = u ^ ((state >> 2) & 1);

            /* 校验位 = fb XOR state_bit0 XOR state_bit1 */
            int p = fb ^ (state & 1) ^ ((state >> 1) & 1);

            /* 状态转移 */
            int newState = ((state & 3) << 1) | fb;
            state = newState;

            output.append(p);
        }
        return output;
    };

    /* 提取系统位(两两一组取第一个) */
    QVector<int> sys1(numSymbols, 0);
    QVector<int> sys2(numSymbols, 0);
    for (int i = 0; i < numSymbols; ++i) {
        sys1[i] = bits[i * 2];
        sys2[i] = bits[i * 2 + 1];
    }

    /* 第一个RSC编码器 */
    QVector<int> par1 = rscEncode(sys1, sys2, false);

    /* 交织后的系统位 */
    QVector<int> intSys1(numSymbols, 0);
    QVector<int> intSys2(numSymbols, 0);
    for (int i = 0; i < numSymbols; ++i) {
        int j = interleave(i, numSymbols);
        intSys1[i] = sys1[j];
        intSys2[i] = sys2[j];
    }

    /* 第二个RSC编码器 */
    QVector<int> par2 = rscEncode(intSys1, intSys2, true);

    /* 组装输出: sys1 | sys2 | par1 | par2 */
    for (int i = 0; i < numSymbols; ++i) {
        encoded.append(sys1[i]);
        encoded.append(sys2[i]);
    }
    for (int i = 0; i < numSymbols; ++i)
        encoded.append(par1[i]);
    for (int i = 0; i < numSymbols; ++i)
        encoded.append(par2[i]);

    return encoded;
}

/**
 * @brief SISO解码器 - Log-MAP算法核心
 *
 * 在对数域执行BCJR算法，计算每个比特的后验LLR:
 * LLR_out = LLR_in + channel + extrinsic
 *
 * @param sysLLR 系统位LLR
 * @param parLLR 校验位LLR
 * @param extLLR 先验(外信息)LLR
 * @param numStates 格栅状态数
 * @return 外信息LLR
 */
static QVector<double> logMAP(const QVector<double>& sysLLR,
                               const QVector<double>& parLLR,
                               const QVector<double>& extLLR,
                               int numStates = 8)
{
    const int T = sysLLR.size();
    if (T == 0) return {};

    /* 前向递推(alpha) */
    QVector<QVector<double>> alpha(T + 1, QVector<double>(numStates, -1e10));
    alpha[0][0] = 0.0; /* 初始状态为0 */

    /* 格栅转移表: nextState[state][input] = {nextState, output} */
    auto nextIdx = [](int state, int input) -> int {
        int fb = input ^ ((state >> 2) & 1);
        return ((state & 3) << 1) | fb;
    };

    auto parityOut = [](int state, int input) -> int {
        int fb = input ^ ((state >> 2) & 1);
        return fb ^ (state & 1) ^ ((state >> 1) & 1);
    };

    /* 前向递推 */
    for (int t = 0; t < T; ++t) {
        double sys = sysLLR[t] + ((t < extLLR.size()) ? extLLR[t] : 0.0);
        double par = parLLR[t];

        for (int s = 0; s < numStates; ++s) {
            if (alpha[t][s] < -1e9) continue;

            for (int u = 0; u <= 1; ++u) {
                int ns = nextIdx(s, u);
                int p = parityOut(s, u);

                /* 分支度量 */
                double bm = 0.0;
                bm += (u == 1) ? sys * 0.5 : -sys * 0.5;
                bm += (p == 1) ? par * 0.5 : -par * 0.5;

                /* Log-MAP: alpha[t+1][ns] = log-sum-exp(alpha[t][s] + bm) */
                double val = alpha[t][s] + bm;
                alpha[t + 1][ns] = (val > alpha[t + 1][ns]) ?
                    alpha[t + 1][ns] + qLn(1.0 + qExp(alpha[t + 1][ns] - val)) :
                    val + qLn(1.0 + qExp(val - alpha[t + 1][ns]));
            }
        }
    }

    /* 后向递推(beta) */
    QVector<QVector<double>> beta(T + 1, QVector<double>(numStates, -1e10));
    beta[T][0] = 0.0; /* 终止状态为0 */

    for (int t = T - 1; t >= 0; --t) {
        double sys = sysLLR[t] + ((t < extLLR.size()) ? extLLR[t] : 0.0);
        double par = parLLR[t];

        for (int s = 0; s < numStates; ++s) {
            for (int u = 0; u <= 1; ++u) {
                int ns = nextIdx(s, u);
                int p = parityOut(s, u);

                double bm = 0.0;
                bm += (u == 1) ? sys * 0.5 : -sys * 0.5;
                bm += (p == 1) ? par * 0.5 : -par * 0.5;

                double val = beta[t + 1][ns] + bm;
                beta[t][s] = (val > beta[t][s]) ?
                    beta[t][s] + qLn(1.0 + qExp(beta[t][s] - val)) :
                    val + qLn(1.0 + qExp(val - beta[t][s]));
            }
        }
    }

    /* 计算外信息LLR */
    QVector<double> extrinsic(T, 0.0);
    for (int t = 0; t < T; ++t) {
        double sys = sysLLR[t] + ((t < extLLR.size()) ? extLLR[t] : 0.0);
        double par = parLLR[t];

        double lMax = -1e10;
        double lMin = -1e10;

        for (int s = 0; s < numStates; ++s) {
            if (alpha[t][s] < -1e9) continue;

            for (int u = 0; u <= 1; ++u) {
                int ns = nextIdx(s, u);
                int p = parityOut(s, u);

                double bm = 0.0;
                bm += (p == 1) ? par * 0.5 : -par * 0.5;

                double llr = alpha[t][s] + bm + beta[t + 1][ns];

                if (u == 1) lMax = qMax(lMax, llr);
                else lMin = qMax(lMin, llr);
            }
        }

        /* 外信息 = 后验LLR - 系统LLR - 先验LLR */
        extrinsic[t] = lMax - lMin - sys;
    }

    return extrinsic;
}

/**
 * @brief CRC校验(简化8位CRC)
 * @param bits 输入比特
 * @return true校验通过
 */
static bool checkCRC(const QVector<int>& bits)
{
    quint8 crc = 0xFF;
    for (int b : bits) {
        quint8 bit = static_cast<quint8>(b);
        quint8 fb = (crc ^ bit) & 1;
        crc >>= 1;
        if (fb) crc ^= 0x8C;
    }
    return crc == 0;
}

/**
 * @brief Turbo解码器 - 迭代MAP解码
 *
 * 在两个SISO解码器之间传递外信息进行迭代:
 * 1. 解码器1处理原始顺序数据
 * 2. 外信息交织后传入解码器2
 * 3. 解码器2处理交织顺序数据
 * 4. 外信息解交织后返回解码器1
 *
 * @param llr 信道LLR输入
 * @return 硬判决输出比特
 */
QVector<int> TurboCode3::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    const int totalLLR = llr.size();
    const int numSymbols = totalLLR / 4;
    const int infoLen = numSymbols * 2;

    QVector<int> result(infoLen, 0);

    if (numSymbols < 1) {
        m_stats.totalDecodes++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);
        return result;
    }

    /* 分解LLR: sys1 | sys2 | par1 | par2 */
    QVector<double> sys1LLR(numSymbols), sys2LLR(numSymbols);
    QVector<double> par1LLR(numSymbols), par2LLR(numSymbols);

    for (int i = 0; i < numSymbols; ++i) {
        sys1LLR[i] = llr[i];
        sys2LLR[i] = llr[numSymbols + i];
        par1LLR[i] = llr[2 * numSymbols + i];
        par2LLR[i] = llr[3 * numSymbols + i];
    }

    /* 交织器 */
    auto interleave = [](int idx, int total) -> int {
        int p = 37;
        while (p < total / 2) p = p * 2 + 1;
        return (idx * p + 1) % total;
    };

    /* 交织后的LLR */
    QVector<double> intSys1LLR(numSymbols), intPar2LLR(numSymbols);
    for (int i = 0; i < numSymbols; ++i) {
        int j = interleave(i, numSymbols);
        intSys1LLR[i] = sys1LLR[j];
        intPar2LLR[i] = par2LLR[j];
    }

    /* 迭代解码 */
    QVector<double> ext1(numSymbols, 0.0);
    QVector<double> ext2(numSymbols, 0.0);

    bool crcPass = false;
    int usedIter = 0;

    for (int iter = 0; iter < m_numIter; ++iter) {
        usedIter = iter + 1;

        /* 解码器1 */
        ext1 = logMAP(sys1LLR, par1LLR, ext2);

        /* 交织外信息 */
        QVector<double> intExt1(numSymbols);
        for (int i = 0; i < numSymbols; ++i)
            intExt1[i] = ext1[interleave(i, numSymbols)];

        /* 解码器2 */
        ext2 = logMAP(intSys1LLR, intPar2LLR, intExt1);

        /* 解交织外信息 */
        QVector<double> deintExt2(numSymbols);
        for (int i = 0; i < numSymbols; ++i) {
            int j = interleave(i, numSymbols);
            deintExt2[j] = ext2[i];
        }
        ext2 = deintExt2;

        /* 早停: 硬判决并检查CRC */
        if (m_earlyStop) {
            for (int i = 0; i < numSymbols; ++i) {
                double llr_val = sys1LLR[i] + ext1[i] + ext2[i];
                result[i * 2] = (llr_val > 0) ? 1 : 0;
            }
            for (int i = 0; i < numSymbols; ++i) {
                double llr_val = sys2LLR[i];
                result[i * 2 + 1] = (llr_val > 0) ? 1 : 0;
            }
            if (checkCRC(result)) {
                crcPass = true;
                break;
            }
        }
    }

    /* 最终硬判决 */
    if (!crcPass) {
        for (int i = 0; i < numSymbols; ++i) {
            double llr_val = sys1LLR[i] + ext1[i] + ext2[i];
            result[i * 2] = (llr_val > 0) ? 1 : 0;
            result[i * 2 + 1] = (sys2LLR[i] > 0) ? 1 : 0;
        }
    }

    m_stats.totalDecodes++;
    m_stats.totalBitsProcessed += infoLen;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeComplete(usedIter, crcPass);
    return result;
}

/**
 * @brief 重置所有统计数据
 */
void TurboCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
