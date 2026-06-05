#include "utils/code78/SpinalCode7.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/**
 * @class SpinalCode7
 * @brief Spinal码(脊柱码)编解码器实现
 *
 * Spinal码是一种基于哈希函数的无速率(Rateless)纠错编码:
 * 1. 将消息分为k比特的小块(spline段)
 * 2. 每个消息块通过哈希函数(脊柱函数)生成脊柱值
 * 3. 从每个脊柱值可以产生无限多个编码符号
 * 4. 接收端通过不断接收更多符号来逐步提升解码可靠性
 *
 * 编码结构: spine_0 = hash(seed, msg_0), spine_i = hash(spine_{i-1}, msg_i)
 * 符号生成: symbol_{i,j} = RNG(spine_i, j) 映射到BPSK
 *
 * 解码使用气泡(Bubble)解码器: 维护一个宽度受限的搜索树，
 * 每层保留beamWidth个最优候选路径，通过欧氏距离排序剪枝。
 * 复杂度: O(beamWidth * 2^k * numSpines)
 */

/**
 * @brief 构造函数，初始化脊柱码编解码器
 * @param parent 父QObject对象指针
 */
SpinalCode7::SpinalCode7(QObject* parent)
    : QObject(parent)
    , m_k(4)
    , m_B(8)
    , m_seed(0x5A5A5A5A5A5A5A5AULL)
{
}

/**
 * @brief 初始化脊柱码参数
 *
 * 设置脊柱深度k(每段消息比特数)和哈希输出位数B。
 * k决定了搜索空间大小(2^k)，k越大编码效率越高但解码越慢。
 * B决定了每个脊柱值的输出精度。
 *
 * 典型配置: k=4, B=8 (搜索空间16，平衡效率与复杂度)
 *
 * @param k 每段消息的比特数(通常4~6)
 * @param B 哈希函数输出位数(通常8~16)
 * @return true 参数合法，false k或B超出范围
 */
bool SpinalCode7::initialize(int k, int B)
{
    if (k < 1 || k > 16) return false;
    if (B < 1 || B > 32) return false;

    m_k = k;
    m_B = B;
    return true;
}

/**
 * @brief 内部哈希函数
 *
 * 使用MurmurHash3风格的混合函数将脊柱值和消息块
 * 组合为新的脊柱值。采用多轮位移+异或+乘法混合
 * 确保输出具有良好的雪崩效应。
 *
 * @param spine 当前的脊柱值
 * @param msgBits 消息块(整数形式)
 * @return 更新后的脊柱值
 */
static quint64 spinalHash(quint64 spine, int msgBits)
{
    quint64 h = spine ^ static_cast<quint64>(msgBits);
    /// 第一轮混合
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    /// 第二轮混合
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return h;
}

/**
 * @brief 从脊柱值生成编码符号
 *
 * 使用脊柱值作为RNG种子，通过线性同余+混合
 * 生成指定序号的编码符号(BPSK调制值)。
 *
 * @param spine 脊柱值
 * @param symbolIndex 符号序号(同一脊柱可产生多个符号)
 * @param B 输出位数
 * @return BPSK调制后的符号值
 */
static double generateSymbol(quint64 spine, int symbolIndex, int B)
{
    quint64 rng = spine ^ (static_cast<quint64>(symbolIndex) * 0x9E3779B97F4A7C15ULL);
    rng ^= rng >> 17;
    rng *= 0x0FC94E3BF4A8EC29ULL;
    rng ^= rng >> 13;

    /// 取高B位并映射到[-1, +1]
    double val = static_cast<double>((rng >> (64 - B)) & ((1ULL << B) - 1));
    double normalized = val / static_cast<double>((1ULL << B) - 1);
    return normalized * 2.0 - 1.0;
}

/**
 * @brief 编码消息为脊柱码符号序列
 *
 * 编码流程:
 * 1. 将消息按k比特分块
 * 2. 依次对每个块计算新脊柱值: spine_i = hash(spine_{i-1}, msg_i)
 * 3. 从每个脊柱值生成 numSymbols/numSpines 个编码符号
 * 4. 输出BPSK符号序列
 *
 * @param message 输入消息比特序列(0/1)
 * @param numSymbols 需要生成的编码符号总数
 * @return 编码后的BPSK符号序列
 */
QVector<double> SpinalCode7::encode(const QVector<int>& message, int numSymbols)
{
    QElapsedTimer timer;
    timer.start();

    if (message.isEmpty() || numSymbols <= 0) {
        m_timeSum += timer.elapsed();
        return {};
    }

    const int numSpines = (message.size() + m_k - 1) / m_k;
    const int symbolsPerSpine = qMax(1, numSymbols / numSpines);

    quint64 spine = m_seed;
    QVector<double> encoded;
    encoded.reserve(numSpines * symbolsPerSpine);

    for (int s = 0; s < numSpines; ++s) {
        /// 从消息中提取k比特组成整数
        int msgBits = 0;
        for (int b = 0; b < m_k; ++b) {
            int idx = s * m_k + b;
            if (idx < message.size()) {
                msgBits |= ((message[idx] & 1) << b);
            }
        }

        /// 更新脊柱值
        spine = spinalHash(spine, msgBits);

        /// 从当前脊柱值生成多个编码符号
        for (int j = 0; j < symbolsPerSpine; ++j) {
            encoded.append(generateSymbol(spine, j, m_B));
        }
    }

    /// 更新统计信息
    m_stats.totalSymbolsGenerated += encoded.size();
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalDecodingAttempts + qMax(1, m_stats.totalSymbolsGenerated / 100);
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    return encoded;
}

/**
 * @brief 气泡(Bubble)解码器
 *
 * 基于宽度优先搜索的解码算法:
 * 1. 初始化: 一条空路径(spine=seed, cost=0)
 * 2. 对每个脊柱段:
 *    a. 对每条候选路径，尝试所有2^k种消息块
 *    b. 计算每种假设的欧氏距离代价
 *    c. 按代价排序，保留beamWidth条最优路径
 * 3. 最终选择代价最小的路径，提取消息比特
 *
 * 路径度量: cost = sum_i sum_j (received[i,j] - symbol(spine_i, j))^2
 *
 * @param received 接收到的符号序列(含噪声)
 * @param messageLength 原始消息比特长度
 * @param beamWidth 搜索宽度(候选路径数)，默认16
 * @return 解码后的消息比特序列
 */
QVector<int> SpinalCode7::decode(const QVector<double>& received, int messageLength, int beamWidth)
{
    QElapsedTimer timer;
    timer.start();

    if (received.isEmpty() || messageLength <= 0) {
        m_timeSum += timer.elapsed();
        return {};
    }

    beamWidth = qBound(1, beamWidth, 256);
    const int numSpines = (messageLength + m_k - 1) / m_k;

    /// 计算每个脊柱段对应的符号数
    int totalReceivedSpines = received.size() / qMax(1, numSpines);
    if (totalReceivedSpines == 0) totalReceivedSpines = 1;

    /// 候选路径结构
    struct Candidate {
        quint64 spine;              ///< 当前脊柱值
        QVector<int> message;       ///< 已解码的消息比特
        double cost;                ///< 累计欧氏距离代价
    };

    /// 初始化搜索: 一条空路径
    QVector<Candidate> beam;
    beam.append({m_seed, {}, 0.0});

    const int symbolsPerSpine = qMax(1, received.size() / numSpines);
    const int numMsgs = 1 << m_k;  ///< 每段可能的取值数(2^k)

    /// 逐脊柱段解码
    for (int s = 0; s < numSpines; ++s) {
        QVector<Candidate> newBeam;
        newBeam.reserve(beam.size() * numMsgs);

        for (const auto& cand : beam) {
            /// 尝试所有可能的消息块
            for (int msg = 0; msg < numMsgs; ++msg) {
                /// 计算新脊柱值
                quint64 newSpine = spinalHash(cand.spine, msg);

                /// 计算该假设下的欧氏距离代价
                double segmentCost = 0.0;
                for (int j = 0; j < symbolsPerSpine; ++j) {
                    int symIdx = s * symbolsPerSpine + j;
                    if (symIdx < received.size()) {
                        double expected = generateSymbol(newSpine, j, m_B);
                        double diff = received[symIdx] - expected;
                        segmentCost += diff * diff;
                    }
                }

                /// 构造新候选
                Candidate newCand;
                newCand.spine = newSpine;
                newCand.message = cand.message;
                newCand.message.reserve(newCand.message.size() + m_k);

                /// 分解消息比特
                for (int b = 0; b < m_k; ++b) {
                    newCand.message.append((msg >> b) & 1);
                }
                newCand.cost = cand.cost + segmentCost;

                newBeam.append(std::move(newCand));
            }
        }

        /// 按代价排序，保留beamWidth条最优路径
        std::sort(newBeam.begin(), newBeam.end(),
                  [](const Candidate& a, const Candidate& b) {
                      return a.cost < b.cost;
                  });

        if (static_cast<int>(newBeam.size()) > beamWidth) {
            newBeam.resize(beamWidth);
        }

        beam = std::move(newBeam);
    }

    /// 截取到指定消息长度
    QVector<int> decoded(messageLength);
    if (!beam.isEmpty()) {
        for (int i = 0; i < messageLength && i < beam[0].message.size(); ++i) {
            decoded[i] = beam[0].message[i];
        }
    }

    /// 判断是否成功(基于最优路径的代价阈值)
    double bestCost = beam.isEmpty() ? 1e10 : beam[0].cost;
    bool success = bestCost < (numSpines * symbolsPerSpine * 0.5);

    /// 更新统计信息
    m_stats.totalDecodingAttempts++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalDecodingAttempts;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    emit decodingCompleted(messageLength, success);
    return decoded;
}

/**
 * @brief 设置哈希函数种子
 *
 * 改变种子会影响编码和解码的脊柱值链，
 * 编解码双方必须使用相同种子才能正确解码。
 *
 * @param seed 64位哈希种子
 */
void SpinalCode7::setSeed(quint64 seed)
{
    m_seed = seed;
}

/**
 * @brief 获取当前统计数据
 * @return 包含已生成符号数、解码尝试次数和平均耗时的Stats结构
 */
SpinalCode7::Stats SpinalCode7::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 *
 * 将符号计数、解码计数和累计时间归零。
 */
void SpinalCode7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
