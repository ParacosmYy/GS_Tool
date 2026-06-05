#include "SpinalCode12.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化脊柱码编解码引擎
 * @param parent 父对象指针
 */
SpinalCode12::SpinalCode12(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void SpinalCode12::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 简化的哈希函数，用于脊柱码编码
 *
 * 将脊柱值与输入比特组合后映射为高斯近似符号。
 * 使用三角函数构造确定性伪随机映射。
 *
 * @param spineValue 脊柱状态值
 * @param rngSeed 随机种子偏移
 * @return 映射后的符号值
 */
static double spinalHash(int spineValue, int rngSeed)
{
    double x = qSin(spineValue * 12.9898 + rngSeed * 78.233) * 43758.5453;
    return (x - qFloor(x)) * 2.0 - 1.0;
}

/**
 * @brief Spinal码编码
 *
 * 将消息按k比特分组，每组更新脊柱状态并通过哈希函数
 * 映射为符号，重复numPasses次以产生冗余编码符号。
 *
 * @param message 原始消息比特序列
 * @param k 每步输入比特数
 * @param numPasses 编码通过次数
 * @return 编码后的符号序列
 */
QVector<double> SpinalCode12::encode(const QVector<int>& message, int k, int numPasses)
{
    QElapsedTimer timer;
    timer.start();

    if (message.isEmpty() || k <= 0 || numPasses <= 0) {
        emit decodeCompleted(0);
        return {};
    }

    int numSegments = (message.size() + k - 1) / k;
    QVector<double> symbols;

    int spine = 0;
    for (int seg = 0; seg < numSegments; ++seg) {
        /* 提取k比特并转为整数 */
        int bits = 0;
        for (int b = 0; b < k; ++b) {
            int idx = seg * k + b;
            int bitVal = (idx < message.size()) ? ((message[idx] != 0) ? 1 : 0) : 0;
            bits |= (bitVal << b);
        }
        /* 更新脊柱值 */
        spine = spine * 131 + bits + 1;

        /* 生成numPasses个编码符号 */
        for (int p = 0; p < numPasses; ++p) {
            symbols.append(spinalHash(spine, p));
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(symbols.size());
    return symbols;
}

/**
 * @brief 连续解码器（最优但复杂度高）
 *
 * 对每个可能的k比特组合进行穷举搜索，
 * 逐步选择使路径度量最小的候选，最终恢复完整消息。
 *
 * @param received 接收符号序列
 * @param k 每步输入比特数
 * @param messageLength 消息长度
 * @return 解码后的消息比特序列
 */
QVector<int> SpinalCode12::decodeSequential(const QVector<double>& received,
                                             int k, int messageLength)
{
    QElapsedTimer timer;
    timer.start();

    if (received.isEmpty() || k <= 0 || messageLength <= 0) {
        emit decodeCompleted(0);
        return {};
    }

    int numSegments = (messageLength + k - 1) / k;
    int numPasses = received.size() / numSegments;
    int candidates = 1 << k;
    QVector<int> decoded;
    decoded.reserve(messageLength);

    int spine = 0;
    for (int seg = 0; seg < numSegments; ++seg) {
        double bestMetric = 1e18;
        int bestBits = 0;

        for (int c = 0; c < candidates; ++c) {
            int trialSpine = spine * 131 + c + 1;
            double metric = 0.0;
            for (int p = 0; p < numPasses; ++p) {
                int symIdx = seg * numPasses + p;
                if (symIdx < received.size()) {
                    double expected = spinalHash(trialSpine, p);
                    double diff = received[symIdx] - expected;
                    metric += diff * diff;
                }
            }
            if (metric < bestMetric) {
                bestMetric = metric;
                bestBits = c;
            }
        }

        spine = spine * 131 + bestBits + 1;

        for (int b = 0; b < k; ++b) {
            if (decoded.size() < messageLength) {
                decoded.append((bestBits >> b) & 1);
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(decoded.size());
    return decoded;
}

/**
 * @brief 气泡解码器（近似但速度快）
 *
 * 每步仅保留bubbleWidth个最优候选路径，
 * 大幅降低搜索空间，适用于实时场景。
 *
 * @param received 接收符号序列
 * @param k 每步输入比特数
 * @param messageLength 消息长度
 * @param bubbleWidth 保留的候选路径数
 * @return 解码后的消息比特序列
 */
QVector<int> SpinalCode12::decodeBubble(const QVector<double>& received, int k,
                                         int messageLength, int bubbleWidth)
{
    QElapsedTimer timer;
    timer.start();

    if (received.isEmpty() || k <= 0 || messageLength <= 0) {
        emit decodeCompleted(0);
        return {};
    }

    int numSegments = (messageLength + k - 1) / k;
    int numPasses = received.size() / numSegments;
    int candidates = 1 << k;
    bubbleWidth = qMax(1, bubbleWidth);

    /* 路径结构 */
    struct Path {
        int spine;
        QVector<int> bits;
        double metric;
    };

    QVector<Path> paths;
    paths.append({0, {}, 0.0});

    for (int seg = 0; seg < numSegments; ++seg) {
        QVector<Path> newPaths;
        newPaths.reserve(paths.size() * candidates);

        for (const auto& path : paths) {
            for (int c = 0; c < candidates; ++c) {
                int trialSpine = path.spine * 131 + c + 1;
                double metric = path.metric;
                for (int p = 0; p < numPasses; ++p) {
                    int symIdx = seg * numPasses + p;
                    if (symIdx < received.size()) {
                        double expected = spinalHash(trialSpine, p);
                        double diff = received[symIdx] - expected;
                        metric += diff * diff;
                    }
                }
                Path np;
                np.spine = trialSpine;
                np.bits = path.bits;
                for (int b = 0; b < k; ++b) {
                    np.bits.append((c >> b) & 1);
                }
                np.metric = metric;
                newPaths.append(np);
            }
        }

        /* 保留最优的bubbleWidth条路径 */
        std::sort(newPaths.begin(), newPaths.end(),
                  [](const Path& a, const Path& b) { return a.metric < b.metric; });
        if (newPaths.size() > bubbleWidth) {
            newPaths.resize(bubbleWidth);
        }
        paths = newPaths;
    }

    /* 截取到消息长度 */
    QVector<int> result;
    if (!paths.isEmpty()) {
        result = paths.first().bits;
        if (result.size() > messageLength) {
            result.resize(messageLength);
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(result.size());
    return result;
}
