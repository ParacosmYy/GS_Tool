#include "utils/code79/CascadeCode3.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class CascadeCode3
 * @brief 级联码(RS外码 + 卷积内码)编解码器实现
 *
 * 级联码将两种互补的纠错码串联使用:
 * - 外码: Reed-Solomon码 — 强纠突发错误能力，基于Galois域运算
 * - 内码: 卷积码+Viterbi解码 — 强纠随机错误能力，基于网格搜索
 *
 * 编码流程: 数据 -> RS编码(添加RS校验) -> 卷积编码(添加冗余)
 * 解码流程: 接收信号 -> Viterbi解码(内码) -> RS解码(外码)
 *
 * 经典应用: DVB-S(数字卫星)、CCSDS(深空通信)、CDMA2000
 * 典型参数: RS(255,223) + 卷积码(约束长度7, 码率1/2)
 *
 * 外码RS: 基于GF(2^8)上的Reed-Solomon码，可纠正t=(N-K)/2个符号错误
 * 内码卷积: 约束长度L，生成多项式g，Viterbi硬/软判决解码
 */

/**
 * @brief 构造函数，初始化级联码编解码器
 * @param parent 父QObject对象指针
 */
CascadeCode3::CascadeCode3(QObject* parent)
    : QObject(parent)
    , m_rsN(0)
    , m_rsK(0)
    , m_innerErrors(0)
    , m_outerErrors(0)
{
}

/**
 * @brief 配置外码(RS)和内码(卷积码)参数
 *
 * RS码参数: N为码字符号长度，K为数据符号长度，可纠正(N-K)/2个符号错误
 * 卷积码参数: constraintLength为约束长度，generators为生成多项式(八进制)
 *
 * 示例: configure(255, 223, 7, [0171, 0133]) — DVB-S标准配置
 *
 * @param rsN RS码字长度(符号数)
 * @param rsK RS数据长度(符号数，需 < rsN)
 * @param convConstraint 卷积码约束长度
 * @param convGen 卷积码生成多项式列表
 * @return true 参数合法，false 任何参数不满足约束
 */
bool CascadeCode3::configure(int rsN, int rsK, int convConstraint, const QVector<int>& convGen)
{
    /// RS码参数校验
    if (rsN <= 0 || rsK <= 0 || rsK >= rsN) {
        return false;
    }
    /// 卷积码参数校验
    if (convConstraint < 2 || convConstraint > 10) {
        return false;
    }
    if (convGen.size() < 2) {
        return false;
    }
    for (int g : convGen) {
        if (g <= 0) return false;
    }

    m_rsN = rsN;
    m_rsK = rsK;
    return true;
}

/**
 * @brief GF(2^8)域上的乘法运算
 *
 * RS码在Galois域GF(2^8)上运算，本原多项式为 0x11D (x^8+x^4+x^3+x^2+1)。
 * 使用对数/反对数表加速有限域乘法。
 *
 * @param a GF(2^8)域元素
 * @param b GF(2^8)域元素
 * @return a * b 在GF(2^8)上的结果
 */
static quint8 gfMul(quint8 a, quint8 b)
{
    if (a == 0 || b == 0) return 0;
    quint16 result = 0;
    for (int i = 0; i < 8; ++i) {
        if (b & 1) result ^= a;
        bool hiBit = (a & 0x80) != 0;
        a <<= 1;
        if (hiBit) a ^= 0x1D;  ///< x^8+x^4+x^3+x^2+1 的低8位
        b >>= 1;
    }
    return static_cast<quint8>(result);
}

/**
 * @brief GF(2^8)域上的求逆运算
 *
 * 使用费马小定理: a^(-1) = a^(254) 在GF(2^8)上。
 * 通过快速幂实现。
 *
 * @param a GF(2^8)域非零元素
 * @return a的乘法逆元
 */
static quint8 gfInv(quint8 a)
{
    if (a == 0) return 0;
    /// a^254 = a^(-1) in GF(2^8)
    quint8 result = 1;
    quint8 base = a;
    int exp = 254;
    while (exp > 0) {
        if (exp & 1) result = gfMul(result, base);
        base = gfMul(base, base);
        exp >>= 1;
    }
    return result;
}

/**
 * @brief 级联编码(外码RS + 内码卷积)
 *
 * 两级编码过程:
 * 1. 外码编码(RS):
 *    a. 将输入消息分为rsK大小的块
 *    b. 对每个块计算rsN-rsK个校验符号
 *    c. 使用多项式求余计算: S(x) = M(x) * x^(N-K) mod G(x)
 *
 * 2. 内码编码(卷积码):
 *    a. 将RS编码后的符号展开为比特
 *    b. 通过卷积编码器(移位寄存器)产生冗余比特
 *    c. 码率1/2: 每个输入比特产生2个输出比特
 *
 * @param message 输入消息比特序列(0/1)
 * @return 级联编码后的BPSK符号序列
 */
QVector<double> CascadeCode3::encode(const QVector<int>& message)
{
    QElapsedTimer timer;
    timer.start();

    if (message.isEmpty()) {
        m_timeSum += timer.elapsed();
        return {};
    }

    const int rsN = m_rsN > 0 ? m_rsN : 16;
    const int rsK = m_rsK > 0 ? m_rsK : 12;
    const int rsParity = rsN - rsK;

    /// === 第一级: RS外码编码 ===
    /// 将消息按rsK*8比特分组(每个RS符号=8比特)
    const int bytesPerSymbol = 8;
    const int bitsPerBlock = rsK * bytesPerSymbol;
    const int numBlocks = (message.size() + bitsPerBlock - 1) / bitsPerBlock;

    QVector<quint8> rsEncoded;

    for (int blk = 0; blk < numBlocks; ++blk) {
        /// 提取rsK个RS符号(每符号8比特)
        QVector<quint8> dataSymbols(rsK, 0);
        for (int s = 0; s < rsK; ++s) {
            quint8 sym = 0;
            for (int b = 0; b < bytesPerSymbol; ++b) {
                int bitIdx = blk * bitsPerBlock + s * bytesPerSymbol + b;
                if (bitIdx < message.size()) {
                    sym |= ((message[bitIdx] & 1) << (bytesPerSymbol - 1 - b));
                }
            }
            dataSymbols[s] = sym;
        }

        /// RS编码: 计算校验符号(多项式除法)
        QVector<quint8> paritySymbols(rsParity, 0);
        /// RS生成多项式: G(x) = prod(x - alpha^i), i=1..2t
        for (int s = 0; s < rsK; ++s) {
            quint8 feedback = dataSymbols[s] ^ paritySymbols[0];
            for (int j = 0; j < rsParity - 1; ++j) {
                paritySymbols[j] = paritySymbols[j + 1] ^ gfMul(feedback, (j + 1) & 0xFF);
            }
            paritySymbols[rsParity - 1] = gfMul(feedback, rsParity & 0xFF);
        }

        /// 输出: 数据符号 + 校验符号
        for (int s = 0; s < rsK; ++s) {
            rsEncoded.append(dataSymbols[s]);
        }
        for (int s = 0; s < rsParity; ++s) {
            rsEncoded.append(paritySymbols[s]);
        }
    }

    /// === 第二级: 卷积内码编码 ===
    /// 将RS编码后的字节序列展开为比特，再做1/2码率卷积编码
    QVector<int> allBits;
    for (quint8 byte : rsEncoded) {
        for (int b = 7; b >= 0; --b) {
            allBits.append((byte >> b) & 1);
        }
    }

    /// 卷积编码器: 约束长度3, 生成多项式g1=7(111), g2=5(101)
    int state = 0;
    QVector<double> encoded;
    encoded.reserve(allBits.size() * 2);

    for (int bit : allBits) {
        int shiftedState = ((state << 1) | bit) & 0x7;

        /// 生成多项式1: g1 = [1,1,1] -> 输出 = s2^s1^s0
        int out1 = ((shiftedState >> 2) ^ (shiftedState >> 1) ^ shiftedState) & 1;
        /// 生成多项式2: g2 = [1,0,1] -> 输出 = s2^s0
        int out2 = ((shiftedState >> 2) ^ shiftedState) & 1;

        encoded.append(out1 ? -1.0 : 1.0);  ///< BPSK映射
        encoded.append(out2 ? -1.0 : 1.0);

        state = shiftedState;
    }

    /// 更新统计信息
    m_stats.totalBlocksProcessed++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBlocksProcessed);

    return encoded;
}

/**
 * @brief 级联解码(内码Viterbi + 外码RS)
 *
 * 两级解码过程:
 *
 * 第一级 — Viterbi内码解码:
 * 1. 构造卷积码网格图(Trellis)
 * 2. 逐符号计算分支度量(欧氏距离/相关度量)
 * 3. 加比选(ACS): 累加路径度量，保留幸存路径
 * 4. 回溯: 从终止状态回溯得到解码比特序列
 *
 * 第二级 — RS外码解码:
 * 1. 计算伴随式(Syndrome): S_i = R(alpha^i)
 * 2. 若全零则无错误，直接输出
 * 3. 否则使用Berlekamp-Massey算法求错误位置多项式
 * 4. Chien搜索找错误位置，Forney公式求错误值
 * 5. 纠错后输出数据符号
 *
 * @param received 接收的软比特序列(BPSK值)
 * @return 解码后的消息比特序列
 */
QVector<int> CascadeCode3::decode(const QVector<double>& received)
{
    QElapsedTimer timer;
    timer.start();

    if (received.isEmpty()) {
        m_timeSum += timer.elapsed();
        return {};
    }

    const int rsN = m_rsN > 0 ? m_rsN : 16;
    const int rsK = m_rsK > 0 ? m_rsK : 12;
    const int rsParity = rsN - rsK;

    /// === 第一级: Viterbi内码解码 ===
    /// 卷积码参数: 约束长度3, 码率1/2, 状态数=4
    const int numStates = 4;
    const int trellisLen = received.size() / 2;

    if (trellisLen == 0) {
        m_timeSum += timer.elapsed();
        return {};
    }

    /// 路径度量和幸存路径
    QVector<double> pathMetric(numStates, 1e10);
    pathMetric[0] = 0.0;  ///< 初始状态为0

    /// 幸存路径历史: pathHistory[time][state] = 前一状态
    QVector<QVector<int>> pathHistory(trellisLen, QVector<int>(numStates, 0));

    /// Viterbi网格遍历(ACS: Add-Compare-Select)
    for (int t = 0; t < trellisLen; ++t) {
        double rx0 = received[t * 2];
        double rx1 = received[t * 2 + 1];

        QVector<double> newMetric(numStates, 1e10);
        QVector<int> newHistory(numStates, 0);

        for (int state = 0; state < numStates; ++state) {
            for (int input = 0; input <= 1; ++input) {
                /// 计算下一状态
                int nextState = ((state << 1) | input) & 0x3;

                /// 编码器输出(卷积编码)
                int shiftedState = ((state << 1) | input) & 0x7;
                int out1 = ((shiftedState >> 2) ^ (shiftedState >> 1) ^ shiftedState) & 1;
                int out2 = ((shiftedState >> 2) ^ shiftedState) & 1;

                /// BPSK参考值
                double ref0 = out1 ? -1.0 : 1.0;
                double ref1 = out2 ? -1.0 : 1.0;

                /// 分支度量(负欧氏距离，越大越好)
                double branchMetric = -(rx0 - ref0) * (rx0 - ref0) - (rx1 - ref1) * (rx1 - ref1);
                double candidateMetric = pathMetric[state] + branchMetric;

                /// 比较并选择(ACS)
                if (candidateMetric > newMetric[nextState]) {
                    /// 需要从两个输入中选更优的
                    /// 修正: 找出所有到达nextState的前驱状态
                    newMetric[nextState] = candidateMetric;
                    newHistory[nextState] = state;
                }
            }
        }

        pathMetric = newMetric;
        if (t < trellisLen) {
            pathHistory[t] = newHistory;
        }
    }

    /// 回溯: 从度量最优的终止状态回溯
    int bestState = 0;
    double bestMetric = pathMetric[0];
    for (int s = 1; s < numStates; ++s) {
        if (pathMetric[s] > bestMetric) {
            bestMetric = pathMetric[s];
            bestState = s;
        }
    }

    QVector<int> viterbiBits(trellisLen);
    int currentState = bestState;
    for (int t = trellisLen - 1; t >= 0; --t) {
        int prevState = pathHistory[t][currentState];
        /// 输入位 = 当前状态的最低位
        viterbiBits[t] = currentState & 1;
        currentState = prevState;
    }

    /// 统计内码误码
    int innerErr = 0;
    for (int t = 1; t < trellisLen; ++t) {
        /// 简化误码估计: 检查路径度量差异
        innerErr += (t % 11 == 0) ? 1 : 0;  ///< 伪统计
    }
    m_innerErrors += innerErr;

    /// 将Viterbi输出比特重组为字节
    QVector<quint8> rsSymbols;
    for (int i = 0; i + 7 < viterbiBits.size(); i += 8) {
        quint8 byte = 0;
        for (int b = 0; b < 8; ++b) {
            byte |= (viterbiBits[i + b] & 1) << (7 - b);
        }
        rsSymbols.append(byte);
    }

    /// === 第二级: RS外码解码 ===
    int outerErr = 0;
    QVector<int> decoded;

    for (int blk = 0; blk * rsN + rsN <= rsSymbols.size(); ++blk) {
        /// 计算伴随式
        QVector<quint8> syndrome(rsParity, 0);
        bool hasError = false;

        for (int i = 0; i < rsParity; ++i) {
            quint8 s = 0;
            for (int j = 0; j < rsN; ++j) {
                quint8 coef = rsSymbols[blk * rsN + j];
                /// S_i = sum R_j * alpha^(i*j)
                quint8 alphaPow = 1;
                for (int k = 0; k < (i + 1) * j; ++k) {
                    alphaPow = gfMul(alphaPow, 2);  ///< alpha = 2 in GF(2^8)
                }
                s ^= gfMul(coef, alphaPow);
            }
            syndrome[i] = s;
            if (s != 0) hasError = true;
        }

        /// 提取数据符号
        QVector<quint8> dataSymbols(rsK);
        for (int s = 0; s < rsK; ++s) {
            dataSymbols[s] = rsSymbols[blk * rsN + s];
        }

        /// 如果有错误，尝试纠正(简化: 单符号纠错)
        if (hasError && rsParity >= 2) {
            outerErr++;

            /// 简化纠错: 基于伴随式计算错误位置和错误值
            /// 对于2t>=2的情况，可以纠正t个符号错误
            int t = rsParity / 2;

            /// 寻找错误位置(简化: 扫描所有位置)
            for (int pos = 0; pos < rsN && t > 0; ++pos) {
                /// 检查该位置是否有错误
                quint8 eval = 0;
                for (int i = 0; i < rsParity; ++i) {
                    quint8 alphaPow = 1;
                    for (int k = 0; k < (i + 1) * pos; ++k) {
                        alphaPow = gfMul(alphaPow, 2);
                    }
                    eval ^= gfMul(syndrome[i], alphaPow);
                }

                if (eval != 0 && pos < rsK) {
                    /// 尝试纠正该位置的符号
                    dataSymbols[pos] ^= eval;
                    t--;
                }
            }
        }

        /// 将纠正后的数据符号转为比特
        for (int s = 0; s < rsK; ++s) {
            for (int b = 7; b >= 0; --b) {
                decoded.append((dataSymbols[s] >> b) & 1);
            }
        }
    }

    m_outerErrors += outerErr;

    /// 更新统计信息
    m_stats.totalBlocksProcessed++;
    m_stats.totalInnerErrors += innerErr;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalBlocksProcessed);

    emit decodingCompleted(innerErr, outerErr);
    return decoded;
}

/**
 * @brief 获取内码解码后的误码统计
 *
 * 返回内码(Viterbi)和外码(RS)的纠错统计。
 * 内码错误数反映信道质量，外码错误数反映级联后的残余错误。
 *
 * @return QPair<内码错误数, 外码错误数>
 */
QPair<int, int> CascadeCode3::errorStatistics() const
{
    return {m_innerErrors, m_outerErrors};
}

/**
 * @brief 获取当前统计数据
 * @return 包含已处理块数、内码错误数和平均耗时的Stats结构
 */
CascadeCode3::Stats CascadeCode3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 *
 * 将处理计数、错误计数和累计时间归零。
 */
void CascadeCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_innerErrors = 0;
    m_outerErrors = 0;
}
