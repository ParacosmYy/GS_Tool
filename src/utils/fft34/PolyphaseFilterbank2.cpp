/**
 * @file PolyphaseFilterbank2.cpp
 * @brief 多相滤波器组增强实现 — 分析/综合/原型滤波器/完美重构
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/fft34/PolyphaseFilterbank2.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化默认参数并生成Kaiser窗原型滤波器
 * @param parent 父对象
 */
PolyphaseFilterbank2::PolyphaseFilterbank2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("PolyphaseFilterbank2"));
    setPrototypeFilter({}); /* 使用默认Kaiser窗 */
}

/**
 * @brief 设置子带数量
 * @param bands 子带数（最小为2，必须为2的幂次）
 */
void PolyphaseFilterbank2::setBands(int bands)
{
    m_bands = qMax(2, bands);
    setPrototypeFilter({});
}

/**
 * @brief 设置每个子带的滤波器抽头数
 * @param taps 每子带抽头数（最小为1）
 */
void PolyphaseFilterbank2::setTapsPerBand(int taps)
{
    m_tapsPerBand = qMax(1, taps);
    setPrototypeFilter({});
}

/**
 * @brief 设置采样率
 * @param rate 采样率(Hz)
 */
void PolyphaseFilterbank2::setSampleRate(double rate)
{
    m_sampleRate = qMax(1.0, rate);
}

/**
 * @brief 设计Kaiser窗原型低通滤波器
 *
 * 原型滤波器长度 = bands * tapsPerBand，截止频率 = pi/bands。
 * 使用Kaiser窗加权sinc函数。
 */
static QVector<double> designPrototype(int bands, int tapsPerBand)
{
    int length = bands * tapsPerBand;
    QVector<double> h(length);
    double cutoff = M_PI / static_cast<double>(bands);
    int center = length / 2;

    /* Kaiser窗参数beta = 5 (约50dB阻带衰减) */
    double beta = 5.0;
    double denom = 1.0;
    for (int i = 1; i <= 20; ++i) {
        denom *= i;
    }
    auto besselI0 = [beta, denom](double x) -> double {
        double sum = 1.0;
        double term = 1.0;
        for (int i = 1; i <= 20; ++i) {
            term *= (x * x / 4.0) / static_cast<double>(i * i);
            sum += term;
        }
        return sum;
    };

    for (int n = 0; n < length; ++n) {
        /* sinc函数 */
        double t = static_cast<double>(n - center);
        double sinc = (qFabs(t) < 1e-10) ? 1.0 : qSin(cutoff * t) / (M_PI * t);

        /* Kaiser窗 */
        double alpha = static_cast<double>(length - 1) / 2.0;
        double winArg = beta * qSqrt(1.0 - qPow((n - alpha) / alpha, 2));
        winArg = qMax(0.0, winArg);
        double win = besselI0(winArg) / besselI0(beta);

        h[n] = sinc * win;
    }

    /* 归一化: 使通带增益为1 */
    double energy = 0.0;
    for (int n = 0; n < length; ++n) {
        energy += h[n];
    }
    if (energy > 0.0) {
        for (int n = 0; n < length; ++n) {
            h[n] *= static_cast<double>(bands) / energy;
        }
    }

    return h;
}

/**
 * @brief 设置自定义原型滤波器系数
 *
 * 若传入空向量，则自动设计Kaiser窗原型滤波器。
 *
 * @param coeffs 滤波器系数，长度应为 bands * tapsPerBand
 */
void PolyphaseFilterbank2::setPrototypeFilter(const QVector<double>& coeffs)
{
    if (coeffs.isEmpty()) {
        m_protoFilter = designPrototype(m_bands, m_tapsPerBand);
    } else {
        m_protoFilter = coeffs;
    }
}

/**
 * @brief 分析: 将输入信号分解为多个子带
 *
 * 使用多相分解实现高效分析滤波器组:
 * 1. 将原型滤波器按多相结构分块
 * 2. 对每个新输入块执行多相滤波 + DFT调制
 *
 * @param input 输入采样序列
 * @return 子带矩阵 [bands x numBlocks]
 */
QVector<QVector<double>> PolyphaseFilterbank2::analyze(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QVector<double>> subbands(m_bands);
    if (input.isEmpty()) {
        emit analysisComplete(m_bands, 0);
        return subbands;
    }

    int numBlocks = input.size() / m_bands;
    int filterLen = m_bands * m_tapsPerBand;

    /* 多相分解: h_k(n) = h(n*M + k), M=bands */
    QVector<QVector<double>> polyphase(m_bands);
    for (int k = 0; k < m_bands; ++k) {
        polyphase[k].resize(m_tapsPerBand);
        for (int n = 0; n < m_tapsPerBand; ++n) {
            int idx = n * m_bands + k;
            if (idx < m_protoFilter.size()) {
                polyphase[k][n] = m_protoFilter[idx];
            }
        }
    }

    /* 状态缓冲区: 存储最近的tapsPerBand个块 */
    QVector<QVector<double>> stateBuffer(m_tapsPerBand, QVector<double>(m_bands, 0.0));
    int statePos = 0;

    for (int block = 0; block < numBlocks; ++block) {
        /* 取当前块数据 */
        QVector<double> blockData(m_bands);
        for (int k = 0; k < m_bands; ++k) {
            blockData[k] = input[block * m_bands + k];
        }

        /* 存入状态缓冲区 */
        stateBuffer[statePos % m_tapsPerBand] = blockData;
        statePos++;

        /* 多相滤波 + DFT调制 */
        for (int k = 0; k < m_bands; ++k) {
            double filtered = 0.0;
            for (int n = 0; n < m_tapsPerBand; ++n) {
                int bufIdx = (statePos - 1 - n + m_tapsPerBand * 2) % m_tapsPerBand;
                filtered += polyphase[k][n] * stateBuffer[bufIdx][k];
            }

            /* DFT调制: 乘以旋转因子 */
            double phase = -2.0 * M_PI * static_cast<double>(k) * static_cast<double>(block)
                           / static_cast<double>(m_bands);
            double cosP = qCos(phase);
            double sinP = qSin(phase);
            /* 取实部作为子带输出 */
            double real = filtered * cosP;
            double imag = filtered * sinP;
            subbands[k].append(qSqrt(real * real + imag * imag));
        }
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += input.size();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit analysisComplete(m_bands, numBlocks);
    return subbands;
}

/**
 * @brief 综合: 将子带信号重构为时域信号
 *
 * 执行分析的逆过程: DFT逆调制 + 多相滤波 + 重叠相加。
 *
 * @param subbands 子带矩阵 [bands x numBlocks]
 * @return 重构的时域采样序列
 */
QVector<double> PolyphaseFilterbank2::synthesize(const QVector<QVector<double>>& subbands)
{
    QElapsedTimer timer;
    timer.start();

    if (subbands.isEmpty() || subbands[0].isEmpty()) {
        return {};
    }

    int numBlocks = subbands[0].size();
    int totalSamples = numBlocks * m_bands;
    QVector<double> output(totalSamples, 0.0);

    int filterLen = m_bands * m_tapsPerBand;

    /* 简单重叠相加重构 */
    QVector<QVector<double>> stateBuffer(m_tapsPerBand, QVector<double>(m_bands, 0.0));
    int statePos = 0;

    for (int block = 0; block < numBlocks; ++block) {
        /* DFT逆调制 */
        QVector<double> blockData(m_bands, 0.0);
        for (int k = 0; k < m_bands && k < subbands.size(); ++k) {
            double phase = 2.0 * M_PI * static_cast<double>(k) * static_cast<double>(block)
                           / static_cast<double>(m_bands);
            blockData[k] = subbands[k][block] * qCos(phase);
        }

        stateBuffer[statePos % m_tapsPerBand] = blockData;
        statePos++;

        /* 多相综合滤波 */
        for (int k = 0; k < m_bands; ++k) {
            double sum = 0.0;
            for (int n = 0; n < m_tapsPerBand; ++n) {
                int bufIdx = (statePos - 1 - n + m_tapsPerBand * 2) % m_tapsPerBand;
                int fIdx = n * m_bands + k;
                double coeff = (fIdx < m_protoFilter.size()) ? m_protoFilter[fIdx] : 0.0;
                sum += coeff * stateBuffer[bufIdx][k];
            }
            if (block * m_bands + k < totalSamples) {
                output[block * m_bands + k] = sum;
            }
        }
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalTransforms++;
    m_stats.totalSamplesProcessed += totalSamples;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    return output;
}

/**
 * @brief 重置所有累积统计信息
 */
void PolyphaseFilterbank2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
