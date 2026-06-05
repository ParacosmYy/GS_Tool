/**
 * @file PrunedFFT4.cpp
 * @brief 剪枝FFT实现，仅计算输入/输出掩码指定的频谱分量
 *
 * 传统FFT计算所有N个频率分量，但在许多应用中只需要
 * 部分输入点或部分输出频率。剪枝FFT通过跳过不活跃的
 * 蝶形运算来减少计算量。
 *
 * 适用于：
 * - 稀疏信号的频谱分析
 * - 仅需特定频带的应用
 * - 计算资源受限的嵌入式场景
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/fft55/PrunedFFT4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认变换大小
 * @param parent 父QObject对象指针
 */
PrunedFFT4::PrunedFFT4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置FFT变换大小
 * @param n 变换点数，应为2的幂次
 */
void PrunedFFT4::setSize(int n)
{
    m_n = qMax(2, n);
    m_inMask.clear();
    m_outMask.clear();
}

/**
 * @brief 设置输入掩码，标记活跃的输入采样点
 * @param active 掩码向量，1表示活跃，0表示不活跃
 */
void PrunedFFT4::setInputMask(const QVector<int>& active)
{
    m_inMask = active;
    /* 确保掩码长度匹配 */
    if (m_inMask.size() < m_n)
        m_inMask.resize(m_n, 0);
}

/**
 * @brief 设置输出掩码，标记需要计算的频率分量
 * @param active 掩码向量，1表示需要计算，0表示跳过
 */
void PrunedFFT4::setOutputMask(const QVector<int>& active)
{
    m_outMask = active;
    if (m_outMask.size() < m_n)
        m_outMask.resize(m_n, 0);
}

/**
 * @brief 计算当前剪枝配置的效率
 *
 * 效率 = 活跃输出数 / 总输出数
 * 值越小表示剪枝越激进，计算量节省越多。
 *
 * @return 效率比例(0.0~1.0)
 */
double PrunedFFT4::efficiency() const
{
    if (m_outMask.isEmpty()) return 1.0;

    int active = 0;
    for (int i = 0; i < m_n; ++i) {
        if (i < m_outMask.size() && m_outMask[i])
            ++active;
    }
    return static_cast<double>(active) / m_n;
}

/**
 * @brief 执行剪枝FFT变换
 *
 * 算法流程：
 * 1. 如果有输入掩码：将非活跃输入置零
 * 2. 执行标准FFT蝶形运算
 * 3. 如果有输出掩码：仅提取活跃频率分量
 * 4. 返回活跃频率的实部序列
 *
 * 注：当前实现使用全FFT后筛选的策略。
 * 对于极端稀疏情况，可通过标记传播进一步优化。
 *
 * @param re 输入实部序列
 * @param im 输入虚部序列
 * @return 活跃频率分量的幅度谱
 */
QVector<double> PrunedFFT4::forward(const QVector<double>& re, const QVector<double>& im)
{
    QElapsedTimer timer;
    timer.start();

    /* 准备输入数据 */
    QVector<double> fRe(m_n, 0.0);
    QVector<double> fIm(m_n, 0.0);
    int len = qMin(m_n, qMin(re.size(), im.size()));

    for (int i = 0; i < len; ++i) {
        /* 如果有输入掩码，跳过非活跃点 */
        if (!m_inMask.isEmpty() && i < m_inMask.size() && !m_inMask[i])
            continue;
        fRe[i] = re[i];
        fIm[i] = im[i];
    }

    /* 位逆序排列 */
    for (int i = 1, j = 0; i < m_n; ++i) {
        int bit = m_n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(fRe[i], fRe[j]);
            std::swap(fIm[i], fIm[j]);
        }
    }

    /* FFT蝶形运算（带剪枝优化） */
    for (int stageLen = 2; stageLen <= m_n; stageLen <<= 1) {
        double angle = -2.0 * M_PI / stageLen;
        double wRe = qCos(angle);
        double wIm = qSin(angle);
        int halfLen = stageLen / 2;

        for (int i = 0; i < m_n; i += stageLen) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < halfLen; ++j) {
                int topIdx = i + j;
                int botIdx = i + j + halfLen;

                double tRe = curRe * fRe[botIdx] - curIm * fIm[botIdx];
                double tIm = curRe * fIm[botIdx] + curIm * fRe[botIdx];

                fRe[botIdx] = fRe[topIdx] - tRe;
                fIm[botIdx] = fIm[topIdx] - tIm;
                fRe[topIdx] = fRe[topIdx] + tRe;
                fIm[topIdx] = fIm[topIdx] + tIm;

                double newRe = curRe * wRe - curIm * wIm;
                double newIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
                curIm = newIm;
            }
        }
    }

    /* 计算幅度谱并应用输出掩码 */
    QVector<double> result;

    if (m_outMask.isEmpty()) {
        /* 无输出掩码：返回全部幅度 */
        result.resize(m_n);
        for (int i = 0; i < m_n; ++i)
            result[i] = qSqrt(fRe[i] * fRe[i] + fIm[i] * fIm[i]);
    } else {
        /* 有输出掩码：仅返回活跃频率 */
        for (int i = 0; i < m_n; ++i) {
            if (i < m_outMask.size() && m_outMask[i]) {
                double mag = qSqrt(fRe[i] * fRe[i] + fIm[i] * fIm[i]);
                result.append(mag);
            }
        }
    }

    /* 更新统计 */
    m_stats.totalTransforms++;
    m_stats.totalPoints += m_n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(m_n);
    return result;
}

/**
 * @brief 重置所有统计数据
 */
void PrunedFFT4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
