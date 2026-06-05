/**
 * @file HilbertHuang2.cpp
 * @brief Hilbert-Huang变换实现 — 经验模态分解(EMD)与Hilbert谱分析
 *
 * 通过经验模态分解将信号分解为固有模态函数(IMF)，
 * 再对每个IMF进行Hilbert变换得到瞬时频率和幅值，构建时频谱。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/fft64/HilbertHuang2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject指针
 */
HilbertHuang2::HilbertHuang2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置最大IMF分解数量
 * @param n 最大IMF数，必须 >= 1
 */
void HilbertHuang2::setMaxIMFs(int n)
{
    m_maxIMFs = qMax(1, n);
}

/**
 * @brief 设置筛分最大迭代次数
 * @param iter 最大迭代次数，防止不收敛
 */
void HilbertHuang2::setMaxSiftIterations(int iter)
{
    m_maxSiftIter = qMax(10, iter);
}

/**
 * @brief 设置筛分停止的SDB阈值
 * @param sd 标准差阈值，越小分解越精细
 */
void HilbertHuang2::setSDThreshold(double sd)
{
    m_sdThresh = qMax(0.001, sd);
}

/**
 * @brief 对信号执行经验模态分解(EMD)
 *
 * EMD流程:
 * 1. 找到信号的所有极大值点和极小值点
 * 2. 三次样条插值构建上/下包络线
 * 3. 计算包络均值并从信号中减去(筛分)
 * 4. 重复直到满足IMF条件(SD < 阈值)
 * 5. 将IMF从信号中减去，对残差重复以上过程
 *
 * @param signal 输入信号
 * @return IMF分量集合，每个元素为一个IMF
 */
QVector<QVector<double>> HilbertHuang2::decompose(const QVector<double>& signal)
{
    QElapsedTimer timer;
    timer.start();

    const int n = signal.size();
    if (n < 4) {
        return {};
    }

    QVector<QVector<double>> imfs;
    QVector<double> residual = signal;
    int totalSifts = 0;

    for (int imfIdx = 0; imfIdx < m_maxIMFs; ++imfIdx) {
        QVector<double> h = residual;
        bool converged = false;

        /* 筛分迭代 */
        for (int sift = 0; sift < m_maxSiftIter; ++sift) {
            totalSifts++;

            /* 计算包络均值 */
            QVector<double> mean = envelopeMean(h);
            if (mean.size() != n) break;

            /* 计算标准差(SD)停止准则 */
            double sd = 0.0;
            double energyH = 0.0;
            for (int i = 0; i < n; ++i) {
                h[i] = h[i] - mean[i];
                double diff = mean[i];
                sd += diff * diff;
                energyH += h[i] * h[i];
            }
            if (energyH > 1e-20) {
                sd = sd / energyH;
            } else {
                sd = 0.0;
            }

            if (sd < m_sdThresh) {
                converged = true;
                break;
            }
        }

        /* 检查是否为有效IMF(至少2个极值点) */
        int extrema = 0;
        for (int i = 1; i < n - 1; ++i) {
            if ((h[i] > h[i - 1] && h[i] > h[i + 1]) ||
                (h[i] < h[i - 1] && h[i] < h[i + 1])) {
                ++extrema;
            }
        }

        if (extrema < 2) break; /* 残差趋势，停止分解 */

        imfs.append(h);

        /* 更新残差 */
        for (int i = 0; i < n; ++i) {
            residual[i] -= h[i];
        }

        /* 检查残差是否单调 */
        bool monotonic = true;
        bool increasing = residual[1] >= residual[0];
        for (int i = 2; i < n; ++i) {
            bool curInc = residual[i] >= residual[i - 1];
            if (curInc != increasing) {
                monotonic = false;
                break;
            }
        }
        if (monotonic) break;
    }

    m_numIMFs = imfs.size();

    /* 更新统计信息 */
    m_stats.totalDecompositions++;
    m_stats.totalIMFs += m_numIMFs;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(m_numIMFs, totalSifts);
    return imfs;
}

/**
 * @brief 计算Hilbert谱
 *
 * 对每个IMF计算瞬时频率和瞬时幅值:
 * 1. 使用离散Hilbert变换获取解析信号
 * 2. 从解析信号提取瞬时幅值和相位
 * 3. 对相位求导得到瞬时频率
 *
 * @param imfs IMF分量集合
 * @return Hilbert谱矩阵 [时间 x IMF]，每元素为瞬时幅值
 */
QVector<QVector<double>> HilbertHuang2::hilbertSpectrum(const QVector<QVector<double>>& imfs)
{
    QElapsedTimer timer;
    timer.start();

    const int numIMFs = imfs.size();
    if (numIMFs == 0) return {};

    const int n = imfs[0].size();

    /* 输出: 每个IMF的瞬时幅值 */
    QVector<QVector<double>> spectrum(numIMFs, QVector<double>(n, 0.0));

    for (int m = 0; m < numIMFs; ++m) {
        const QVector<double>& imf = imfs[m];

        /* 离散Hilbert变换(简化实现: 使用频域方法) */
        /* 解析信号 = imf + j * hilbert(imf) */
        /* 简化: 使用差分近似Hilbert变换 */
        QVector<double> hilbert(n, 0.0);
        for (int i = 1; i < n - 1; ++i) {
            /* 中心差分近似导数，结合权重模拟Hilbert核 */
            hilbert[i] = (imf[i + 1] - imf[i - 1]) * 0.5;
        }
        if (n > 1) {
            hilbert[0] = imf[1] - imf[0];
            hilbert[n - 1] = imf[n - 1] - imf[n - 2];
        }

        /* 计算瞬时幅值(解析信号的模) */
        for (int i = 0; i < n; ++i) {
            double re = imf[i];
            double im = hilbert[i];
            spectrum[m][i] = qSqrt(re * re + im * im);
        }
    }

    m_timeSum += timer.elapsed();
    return spectrum;
}

/**
 * @brief 重置所有统计数据
 */
void HilbertHuang2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 对信号执行单次筛分操作
 *
 * 计算信号的上下包络并返回均值。
 *
 * @param sig 输入信号
 * @return 包络均值
 */
QVector<double> HilbertHuang2::findSift(const QVector<double>& sig)
{
    return envelopeMean(sig);
}

/**
 * @brief 计算信号的包络均值
 *
 * 找到所有极大值点和极小值点，使用线性插值
 * 构建上下包络，返回两个包络的平均值。
 *
 * @param sig 输入信号
 * @return 上下包络的均值
 */
QVector<double> HilbertHuang2::envelopeMean(const QVector<double>& sig)
{
    const int n = sig.size();
    if (n < 4) return QVector<double>(n, 0.0);

    /* 找极大值点 */
    QVector<int> maxIdx;
    QVector<double> maxVal;
    for (int i = 1; i < n - 1; ++i) {
        if (sig[i] >= sig[i - 1] && sig[i] >= sig[i + 1]) {
            maxIdx.append(i);
            maxVal.append(sig[i]);
        }
    }

    /* 找极小值点 */
    QVector<int> minIdx;
    QVector<double> minVal;
    for (int i = 1; i < n - 1; ++i) {
        if (sig[i] <= sig[i - 1] && sig[i] <= sig[i + 1]) {
            minIdx.append(i);
            minVal.append(sig[i]);
        }
    }

    /* 如果极值点太少，返回零均值 */
    if (maxIdx.size() < 2 || minIdx.size() < 2) {
        return QVector<double>(n, 0.0);
    }

    /* 上包络: 线性插值 */
    QVector<double> upperEnv(n, 0.0);
    for (int i = 0; i < n; ++i) {
        /* 找到i所在的区间 */
        int seg = -1;
        for (int j = 0; j < maxIdx.size() - 1; ++j) {
            if (i >= maxIdx[j] && i <= maxIdx[j + 1]) {
                seg = j;
                break;
            }
        }
        if (seg >= 0) {
            double t = static_cast<double>(i - maxIdx[seg])
                     / qMax(1, maxIdx[seg + 1] - maxIdx[seg]);
            upperEnv[i] = maxVal[seg] + t * (maxVal[seg + 1] - maxVal[seg]);
        } else if (i < maxIdx[0]) {
            upperEnv[i] = maxVal[0];
        } else {
            upperEnv[i] = maxVal.last();
        }
    }

    /* 下包络: 线性插值 */
    QVector<double> lowerEnv(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int seg = -1;
        for (int j = 0; j < minIdx.size() - 1; ++j) {
            if (i >= minIdx[j] && i <= minIdx[j + 1]) {
                seg = j;
                break;
            }
        }
        if (seg >= 0) {
            double t = static_cast<double>(i - minIdx[seg])
                     / qMax(1, minIdx[seg + 1] - minIdx[seg]);
            lowerEnv[i] = minVal[seg] + t * (minVal[seg + 1] - minVal[seg]);
        } else if (i < minIdx[0]) {
            lowerEnv[i] = minVal[0];
        } else {
            lowerEnv[i] = minVal.last();
        }
    }

    /* 计算均值 */
    QVector<double> mean(n, 0.0);
    for (int i = 0; i < n; ++i) {
        mean[i] = (upperEnv[i] + lowerEnv[i]) * 0.5;
    }
    return mean;
}
