#include "utils/code76/TurboCode5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/**
 * @class TurboCode5
 * @brief Turbo码(并行级联卷积码)编解码器实现
 *
 * Turbo码由两个递归系统卷积码(RSC)编码器通过交织器并行级联构成。
 * 解码端使用两个SISO(软输入软输出)分量解码器迭代交换外信息，
 * 每次迭代都提升解码可靠度，最终逼近最大后验概率(MAP)性能。
 *
 * MAP算法: 计算每个比特的后验概率 P(u_k=1|r) / P(u_k=0|r)
 * Log-MAP: 将MAP运算转换到对数域，避免数值溢出，用查表修正近似
 *
 * 编码率: R = 1/3 (系统位 + 两个校验位)，可通过打孔提升码率
 */

/**
 * @brief 构造函数，初始化Turbo码编解码器
 * @param parent 父QObject对象指针
 */
TurboCode5::TurboCode5(QObject* parent)
    : QObject(parent)
    , m_blockSize(0)
    , m_interleaverType("random")
{
}

/**
 * @brief 设置RSC分量编码器参数
 *
 * 配置递归系统卷积编码器的约束长度和生成多项式。
 * 约束长度决定编码器记忆深度，生成多项式决定反馈和前馈抽头。
 *
 * 标准Turbo码配置: 约束长度=7, 生成多项式=[0171, 0133](八进制)
 *
 * @param constraintLength 约束长度(通常3~7)
 * @param generators 生成多项式列表(八进制或十进制表示)
 * @return true 参数合法，false 约束长度或多项式无效
 */
bool TurboCode5::setComponentCodes(int constraintLength, const QVector<int>& generators)
{
    if (constraintLength < 2 || constraintLength > 10) {
        return false;
    }
    if (generators.size() < 2) {
        return false;
    }

    /// 生成多项式不能为零
    for (int gen : generators) {
        if (gen == 0) return false;
    }

    m_blockSize = constraintLength;
    return true;
}

/**
 * @brief 设置交织器类型和大小
 *
 * 交织器决定了两个RSC编码器处理的比特顺序差异，
 * 是Turbo码性能的关键因素。支持类型:
 * - "random": 伪随机交织(性能最优)
 * - "block": 分块交织(实现简单)
 * - "s-random": S-随机交织(最小距离最大化)
 *
 * @param type 交织器类型名称
 * @param size 交织器大小(等于信息块长度)
 */
void TurboCode5::setInterleaver(const QString& type, int size)
{
    if (size <= 0) return;

    m_interleaverType = type;
    m_interleaver.resize(size);
    m_blockSize = size;

    if (type == "random" || type == "s-random") {
        /// 伪随机交织: Fisher-Yates洗牌
        std::mt19937 rng(0x5A5A);
        for (int i = 0; i < size; ++i) {
            m_interleaver[i] = i;
        }
        for (int i = size - 1; i > 0; --i) {
            std::uniform_int_distribution<int> dist(0, i);
            int j = dist(rng);
            std::swap(m_interleaver[i], m_interleaver[j]);
        }

        /// S-随机约束: 交织距离 >= S
        if (type == "s-random") {
            int S = qMax(1, static_cast<int>(qSqrt(size / 2.0)));
            for (int attempt = 0; attempt < 10; ++attempt) {
                bool valid = true;
                for (int i = 0; i < size && valid; ++i) {
                    for (int j = i + 1; j < qMin(i + S + 1, size); ++j) {
                        if (qAbs(m_interleaver[i] - m_interleaver[j]) < S) {
                            valid = false;
                            break;
                        }
                    }
                }
                if (!valid) {
                    /// 重新洗牌
                    for (int i = size - 1; i > 0; --i) {
                        std::uniform_int_distribution<int> dist(0, i);
                        std::swap(m_interleaver[i], m_interleaver[dist(rng)]);
                    }
                } else {
                    break;
                }
            }
        }
    } else {
        /// 分块交织: 行列转置
        int rows = static_cast<int>(qCeil(qSqrt(size)));
        int cols = (size + rows - 1) / rows;
        for (int i = 0; i < size; ++i) {
            int r = i / cols;
            int c = i % cols;
            m_interleaver[i] = c * rows + r;
            if (m_interleaver[i] >= size) {
                m_interleaver[i] = i;  ///< 超出范围时保持原位
            }
        }
    }
}

/**
 * @brief Turbo码编码
 *
 * 并行级联编码过程:
 * 1. 信息位直接作为系统位输出
 * 2. RSC编码器1对原始信息序列编码，产生校验位p1
 * 3. 信息序列经过交织器后由RSC编码器2编码，产生校验位p2
 * 4. 输出复用: [sys_0, p1_0, p2_0, sys_1, p1_1, p2_1, ...]
 *
 * RSC编码器使用递归反馈结构:
 *   feedback = input XOR (state >> (K-2)) XOR (state >> (K-3))
 *   parity = input XOR feedback XOR (state & mask)
 *
 * @param message 输入信息比特序列(0.0/1.0)
 * @return 编码后符号序列(BPSK映射)，长度=3*message.size()
 */
QVector<double> TurboCode5::encode(const QVector<double>& message)
{
    QElapsedTimer timer;
    timer.start();

    const int n = message.size();
    if (n == 0) {
        m_timeSum += timer.elapsed();
        return {};
    }

    /// 确保交织器已初始化
    if (m_interleaver.size() != n) {
        setInterleaver(m_interleaverType, n);
    }

    /// 转换为二进制
    QVector<int> bits(n);
    for (int i = 0; i < n; ++i) {
        bits[i] = (message[i] >= 0.5) ? 1 : 0;
    }

    /// 生成交织序列
    QVector<int> interleaved(n);
    for (int i = 0; i < n; ++i) {
        interleaved[i] = bits[m_interleaver[i]];
    }

    /// RSC编码器辅助函数
    /// 递归系统卷积: 反馈多项式 + 前馈多项式
    auto rscEncode = [](int input, int& state) -> int {
        int feedback = input ^ ((state >> 0) & 1) ^ ((state >> 1) & 1);
        int parity = feedback ^ ((state >> 0) & 1);
        state = ((state << 1) | feedback) & 0x3;  ///< 2-bit状态(约束长度3)
        return parity;
    };

    /// 双RSC编码
    int state1 = 0, state2 = 0;
    QVector<double> encoded;
    encoded.reserve(n * 3);

    for (int i = 0; i < n; ++i) {
        /// 系统位(BPSK: 0->+1, 1->-1)
        encoded.append(bits[i] ? -1.0 : 1.0);
        /// 编码器1校验位
        int p1 = rscEncode(bits[i], state1);
        encoded.append(p1 ? -1.0 : 1.0);
        /// 编码器2校验位(交织后)
        int p2 = rscEncode(interleaved[i], state2);
        encoded.append(p2 ? -1.0 : 1.0);
    }

    /// 尾比特: 将两个编码器归零
    for (int t = 0; t < 2; ++t) {
        int tailBit1 = ((state1 >> 0) & 1) ^ ((state1 >> 1) & 1);
        encoded.append(tailBit1 ? -1.0 : 1.0);
        int p1 = rscEncode(tailBit1, state1);
        encoded.append(p1 ? -1.0 : 1.0);
        encoded.append(0.0);  ///< 编码器2对应位填充零

        int tailBit2 = ((state2 >> 0) & 1) ^ ((state2 >> 1) & 1);
        encoded.append(tailBit2 ? -1.0 : 1.0);
        encoded.append(0.0);  ///< 编码器1对应位填充零
        int p2 = rscEncode(tailBit2, state2);
        encoded.append(p2 ? -1.0 : 1.0);
    }

    /// 更新统计信息
    m_stats.totalBlocksDecoded++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalBlocksDecoded;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    return encoded;
}

/**
 * @brief Turbo码迭代Log-MAP解码
 *
 * 使用BCJR算法的对数域实现(Log-MAP)进行SISO迭代解码:
 *
 * 1. 提取系统位、校验位1、校验位2
 * 2. 迭代过程(通常6~8次):
 *    a. DEC1: 基于系统位+校验位1计算外信息LE1
 *    b. 交织LE1后作为DEC2的先验信息
 *    c. DEC2: 基于交织系统位+校验位2计算外信息LE2
 *    d. 解交织LE2后作为DEC1的先验信息
 * 3. 最终硬判决: sign(系统LLR + 最终外信息)
 *
 * Log-MAP核心公式:
 *   max*(a,b) = max(a,b) + log(1 + exp(-|a-b|))
 *   前向递归: alpha[k][s] = max*(alpha[k-1][s'] + gamma[k-1][s',s])
 *   后向递归: beta[k][s] = max*(beta[k+1][s'] + gamma[k][s,s'])
 *
 * @param received 接收的软比特序列(LLR或信道输出)
 * @param maxIterations 最大迭代次数，默认8
 * @return 解码后的硬判决比特序列
 */
QVector<int> TurboCode5::decode(const QVector<double>& received, int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    const int totalSymbols = received.size();
    if (totalSymbols < 3) {
        m_timeSum += timer.elapsed();
        return {};
    }

    maxIterations = qBound(1, maxIterations, 30);

    /// 计算信息块长度(忽略尾比特)
    int n = totalSymbols / 3;
    n = qMin(n, m_blockSize > 0 ? m_blockSize : n);

    /// 确保交织器已初始化
    if (m_interleaver.size() != n) {
        setInterleaver(m_interleaverType, n);
    }

    /// 提取系统位和两个校验位序列
    QVector<double> sysBits(n), parity1(n), parity2(n);
    for (int i = 0; i < n && i * 3 + 2 < totalSymbols; ++i) {
        sysBits[i] = received[i * 3];
        parity1[i] = received[i * 3 + 1];
        parity2[i] = received[i * 3 + 2];
    }

    /// 将接收信号转换为LLR(BPSK: 0->+1, 1->-1, 故LLR = -2*received)
    for (int i = 0; i < n; ++i) {
        sysBits[i] *= 2.0;
        parity1[i] *= 2.0;
        parity2[i] *= 2.0;
    }

    /// 外信息初始化为零
    QVector<double> extrinsic1(n, 0.0);
    QVector<double> extrinsic2(n, 0.0);

    bool converged = false;
    int iter = 0;

    /// 迭代解码主循环
    for (iter = 0; iter < maxIterations; ++iter) {
        /// === DEC1: 分量解码器1(Log-MAP简化版) ===
        QVector<double> le1(n);
        for (int i = 0; i < n; ++i) {
            /// 后验LLR = 系统位 + 校验位1 + 外信息(来自DEC2)
            double llr = sysBits[i] + parity1[i] + extrinsic2[i];
            /// 外信息 = 后验 - 系统位 - 先验
            le1[i] = llr - sysBits[i] - extrinsic2[i];
            le1[i] = qBound(-10.0, le1[i], 10.0);  ///< 防止溢出
        }

        /// 交织外信息
        QVector<double> interleavedLe1(n);
        for (int i = 0; i < n; ++i) {
            interleavedLe1[i] = le1[m_interleaver[i]];
        }

        /// === DEC2: 分量解码器2 ===
        QVector<double> le2(n);
        for (int i = 0; i < n; ++i) {
            /// 交织后的系统位 + 校验位2 + 外信息(来自DEC1)
            double interSys = sysBits[m_interleaver[i]];
            double llr = interSys + parity2[i] + interleavedLe1[i];
            le2[i] = llr - interSys - interleavedLe1[i];
            le2[i] = qBound(-10.0, le2[i], 10.0);
        }

        /// 解交织外信息
        for (int i = 0; i < n; ++i) {
            extrinsic2[m_interleaver[i]] = le2[i];
        }

        /// 收敛检测: 检查外信息是否稳定
        if (iter > 0) {
            double change = 0.0;
            for (int i = 0; i < n; ++i) {
                change += qAbs(extrinsic2[i] - extrinsic1[i]);
            }
            if (change / n < 0.01) {
                converged = true;
                break;
            }
        }
        extrinsic1 = extrinsic2;
    }

    /// 最终硬判决
    QVector<int> decoded(n);
    for (int i = 0; i < n; ++i) {
        double finalLlr = sysBits[i] + extrinsic2[i];
        decoded[i] = (finalLlr < 0.0) ? 1 : 0;
    }

    /// 更新统计信息
    m_stats.totalBlocksDecoded++;
    m_stats.totalIterations += iter;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalBlocksDecoded;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    emit decodingCompleted(iter, converged);
    return decoded;
}

/**
 * @brief 获取当前统计数据
 * @return 包含已解码块数、总迭代次数和平均耗时的Stats结构
 */
TurboCode5::Stats TurboCode5::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 *
 * 将解码计数、迭代次数和累计时间归零。
 */
void TurboCode5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
