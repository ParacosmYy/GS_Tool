/**
 * @file XoshiroGenerator.cpp
 * @brief Xoshiro256** 伪随机数生成器实现
 */

#include "utils/random2/XoshiroGenerator.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 循环左移(64位) — 内联辅助函数
 * @param x 操作数
 * @param k 移位位数
 * @return 循环左移结果
 */
static inline quint64 rotl64(quint64 x, int k)
{
    return (x << k) | (x >> (64 - k));
}

/** @brief 默认构造函数 @param parent 父对象 */
XoshiroGenerator::XoshiroGenerator(QObject* parent)
    : QObject(parent)
    , m_splitState(0)
    , m_hasSpare(false)
    , m_spare(0.0)
    , m_timeSum(0.0)
{
    /* 使用QRandomGenerator生成高质量种子 */
    seed(static_cast<quint64>(QRandomGenerator::global()->generate64()));
}

/** @brief 手动播种
 *  @param s 种子值
 */
void XoshiroGenerator::seed(quint64 s)
{
    m_splitState = s;
    /* 使用SplitMix64将单个种子扩展为4个独立状态字 */
    m_state[0] = splitmix64();
    m_state[1] = splitmix64();
    m_state[2] = splitmix64();
    m_state[3] = splitmix64();
    m_hasSpare = false;
}

/** @brief SplitMix64生成器(种子扩展)
 *  @return 64位伪随机数
 */
quint64 XoshiroGenerator::splitmix64()
{
    quint64 z = (m_splitState += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

/** @brief 生成下一个64位随机数
 *  @return [0, 2^64-1] 范围的随机整数
 */
quint64 XoshiroGenerator::next()
{
    /* Xoshiro256**核心算法 */
    const quint64 result = rotl64(m_state[1] * 5, 7) * 9;
    const quint64 t = m_state[1] << 17;

    m_state[2] ^= m_state[0];
    m_state[3] ^= m_state[1];
    m_state[1] ^= m_state[2];
    m_state[0] ^= m_state[3];

    m_state[2] ^= t;
    m_state[3] = rotl64(m_state[3], 45);

    ++m_stats.totalGenerated;
    return result;
}

/** @brief 生成[0, 1)范围的随机双精度浮点数
 *  @return [0.0, 1.0) 随机浮点数
 */
double XoshiroGenerator::nextDouble()
{
    /* 取高53位转换为双精度, 保证精度 */
    return static_cast<double>(next() >> 11) * 0x1.0p-53;
}

/** @brief 生成[min, max]范围的随机整数
 *  @param min 最小值
 *  @param max 最大值
 *  @return [min, max] 随机整数
 */
int XoshiroGenerator::nextInt(int min, int max)
{
    if (min >= max) return min;
    quint64 range = static_cast<quint64>(max - min + 1);
    return min + static_cast<int>(next() % range);
}

/** @brief 生成标准正态分布随机数(Box-Muller变换)
 *  @return N(0,1) 随机数
 */
double XoshiroGenerator::nextGaussian()
{
    if (m_hasSpare) {
        m_hasSpare = false;
        return m_spare;
    }

    /* Box-Muller变换: 从两个均匀随机数生成两个正态随机数 */
    double u1, u2;
    do {
        u1 = nextDouble();
    } while (u1 < 1e-15); /* 避免log(0) */
    u2 = nextDouble();

    double mag = qSqrt(-2.0 * qLn(u1));
    double z0 = mag * qCos(2.0 * M_PI * u2);
    double z1 = mag * qSin(2.0 * M_PI * u2);

    m_spare = z1;
    m_hasSpare = true;

    return z0;
}

/** @brief Fisher-Yates原位洗牌
 *  @param data 待洗牌数据
 */
void XoshiroGenerator::shuffle(QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    int n = data.size();
    for (int i = n - 1; i > 0; --i) {
        int j = nextInt(0, i);
        std::swap(data[i], data[j]);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;

    emit shuffleCompleted(n);
}

/** @brief 无放回采样
 *  @param population 总体数据
 *  @param k 采样数量
 *  @return 采样结果
 */
QVector<double> XoshiroGenerator::sample(const QVector<double>& population,
                                         int k)
{
    QElapsedTimer timer;
    timer.start();

    int n = population.size();
    if (k <= 0 || n == 0) return QVector<double>();
    if (k >= n) {
        QVector<double> result = population;
        shuffle(result);
        return result;
    }

    /* 蓄水池采样算法(Algorithm R) */
    QVector<double> result;
    result.reserve(k);
    for (int i = 0; i < k; ++i) {
        result.append(population[i]);
    }

    for (int i = k; i < n; ++i) {
        int j = nextInt(0, i);
        if (j < k) {
            result[j] = population[i];
        }
    }

    /* 打乱结果顺序保证随机性 */
    shuffle(result);

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;

    emit sampleCompleted(k);
    return result;
}

/** @brief 重置统计信息 */
void XoshiroGenerator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
