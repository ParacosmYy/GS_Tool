/**
 * @file SeededRandom.cpp
 * @brief PCG随机数生成器实现
 */

#include "SeededRandom.h"
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <cmath>
#include <algorithm>

SeededRandom::SeededRandom(QObject* parent)
    : QObject(parent)
    , m_state(0)
    , m_inc(0)
    , m_hasSpare(false)
    , m_spare(0.0)
    , m_timeSum(0.0)
{
    /* 使用QRandomGenerator初始化 */
    seed(QRandomGenerator::global()->generate64());
}

void SeededRandom::seed(quint64 seed)
{
    m_state = 0;
    m_inc = (seed << 1) | 1;
    nextUInt32();
    m_state += seed;
    nextUInt32();
    m_hasSpare = false;
}

quint32 SeededRandom::nextUInt32()
{
    quint64 oldState = m_state;
    m_state = oldState * 6364136223846793005ULL + m_inc;

    quint32 xorShifted = static_cast<quint32>(((oldState >> 18) ^ oldState) >> 27);
    quint32 rot = static_cast<quint32>(oldState >> 59);

    return (xorShifted >> rot) | (xorShifted << ((-static_cast<quint32>(rot)) & 31));
}

quint64 SeededRandom::nextUInt64()
{
    return (static_cast<quint64>(nextUInt32()) << 32) | nextUInt32();
}

double SeededRandom::nextDouble()
{
    return static_cast<double>(nextUInt32()) / 4294967296.0;
}

int SeededRandom::nextInt(int min, int max)
{
    if (min >= max) return min;
    quint32 range = static_cast<quint32>(max - min + 1);
    return min + static_cast<int>(nextUInt32() % range);
}

double SeededRandom::nextGaussian(double mean, double stddev)
{
    QElapsedTimer timer;
    timer.start();

    if (m_hasSpare) {
        m_hasSpare = false;
        double result = mean + stddev * m_spare;
        m_stats.totalGenerated++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerated;
        return result;
    }

    double u1 = nextDouble();
    double u2 = nextDouble();
    u1 = qMax(u1, 1e-15);

    double mag = stddev * std::sqrt(-2.0 * std::log(u1));
    double z0 = mag * std::cos(2.0 * M_PI * u2);
    m_spare = mag * std::sin(2.0 * M_PI * u2);
    m_hasSpare = true;

    m_stats.totalGenerated++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerated;

    return mean + z0;
}

double SeededRandom::choice(const QVector<double>& population)
{
    if (population.isEmpty()) return 0.0;
    return population[nextInt(0, population.size() - 1)];
}

QVector<double> SeededRandom::sample(const QVector<double>& population, int k)
{
    QElapsedTimer timer;
    timer.start();

    if (k <= 0 || population.isEmpty()) return {};

    k = qMin(k, population.size());
    QVector<double> result;
    result.reserve(k);

    /* 蓄水池抽样 */
    for (int i = 0; i < k; ++i)
        result.append(population[i]);

    for (int i = k; i < population.size(); ++i) {
        int j = nextInt(0, i);
        if (j < k)
            result[j] = population[i];
    }

    m_stats.totalGenerated += k;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerated;

    return result;
}

void SeededRandom::shuffle(QVector<double>& data)
{
    for (int i = data.size() - 1; i > 0; --i) {
        int j = nextInt(0, i);
        std::swap(data[i], data[j]);
    }
    m_stats.totalGenerated += data.size();
}

QByteArray SeededRandom::nextBytes(int length)
{
    QByteArray bytes(length, 0);
    for (int i = 0; i < length; i += 4) {
        quint32 val = nextUInt32();
        int remaining = qMin(4, length - i);
        for (int j = 0; j < remaining; ++j)
            bytes[i + j] = static_cast<char>((val >> (8 * j)) & 0xFF);
    }
    m_stats.totalGenerated += length;
    return bytes;
}

SeededRandom::Stats SeededRandom::stats() const { return m_stats; }

void SeededRandom::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
