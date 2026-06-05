/**
 * @file RecursiveDFT2.cpp
 * @brief 递归DFT实现，基于时间抽取(DIT)的Cooley-Tukey算法
 *
 * 实现了递归式离散傅里叶变换(DFT)及其逆变换。
 * 采用分治策略将N点DFT分解为两个N/2点DFT，
 * 递归分解直到基数为2或4的简单情况。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/fft54/RecursiveDFT2.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认变换大小
 * @param parent 父QObject对象指针
 */
RecursiveDFT2::RecursiveDFT2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置DFT变换大小
 * @param n 变换点数，理想为2的幂次
 */
void RecursiveDFT2::setSize(int n)
{
    m_n = qMax(2, n);
}

/**
 * @brief 递归DIT-FFT核心算法
 *
 * 将N点DFT分解为偶数和奇数索引两个N/2点DFT，
 * 然后通过旋转因子(Twiddle Factor)合并结果。
 * 递归终止条件为N=1时直接复制。
 *
 * @param re 输入实部序列
 * @param im 输入虚部序列
 * @param outRe 输出实部序列
 * @param outIm 输出虚部序列
 * @param n 当前子问题大小
 * @param stride 输入数据步长
 */
void RecursiveDFT2::ditfft(const QVector<double>& re, const QVector<double>& im,
                            QVector<double>& outRe, QVector<double>& outIm,
                            int n, int stride)
{
    if (n == 1) {
        outRe[0] = re[0];
        outIm[0] = im[0];
        return;
    }

    /* 分配子问题输出空间 */
    QVector<double> evenRe(n / 2), evenIm(n / 2);
    QVector<double> oddRe(n / 2), oddIm(n / 2);

    /* 递归计算偶数和奇数索引的DFT */
    /* 构建子序列：步长翻倍 */
    QVector<double> subRe(n / 2), subIm(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        subRe[i] = re[2 * i * stride];
        subIm[i] = im[2 * i * stride];
    }
    ditfft(subRe, subIm, evenRe, evenIm, n / 2, stride);

    for (int i = 0; i < n / 2; ++i) {
        subRe[i] = re[(2 * i + 1) * stride];
        subIm[i] = im[(2 * i + 1) * stride];
    }
    ditfft(subRe, subIm, oddRe, oddIm, n / 2, stride);

    /* 蝶形合并：利用旋转因子 */
    for (int k = 0; k < n / 2; ++k) {
        double angle = -2.0 * M_PI * k / n;
        double twRe = qCos(angle);
        double twIm = qSin(angle);

        double tRe = twRe * oddRe[k] - twIm * oddIm[k];
        double tIm = twRe * oddIm[k] + twIm * oddRe[k];

        outRe[k] = evenRe[k] + tRe;
        outIm[k] = evenIm[k] + tIm;
        outRe[k + n / 2] = evenRe[k] - tRe;
        outIm[k + n / 2] = evenIm[k] - tIm;
    }
}

/**
 * @brief 正向DFT变换
 *
 * 将时域信号变换到频域。使用递归DIT-FFT算法。
 * 如果输入长度不足m_n，自动零填充。
 *
 * @param re 输入信号实部
 * @param im 输入信号虚部
 * @return QPair<频域实部, 频域虚部>
 */
QPair<QVector<double>, QVector<double>> RecursiveDFT2::forward(const QVector<double>& re,
                                                                 const QVector<double>& im)
{
    QElapsedTimer timer;
    timer.start();

    /* 准备输入，零填充至m_n */
    QVector<double> inRe(m_n, 0.0);
    QVector<double> inIm(m_n, 0.0);
    int len = qMin(m_n, qMin(re.size(), im.size()));
    for (int i = 0; i < len; ++i) {
        inRe[i] = re[i];
        inIm[i] = im[i];
    }

    QVector<double> outRe(m_n, 0.0);
    QVector<double> outIm(m_n, 0.0);

    ditfft(inRe, inIm, outRe, outIm, m_n, 1);

    /* 更新统计 */
    m_stats.totalTransforms++;
    m_stats.totalPoints += m_n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(m_n);
    return {outRe, outIm};
}

/**
 * @brief 逆DFT变换
 *
 * 将频域信号变换回时域。通过共轭-正变换-共轭的方法实现逆变换。
 * 结果除以N进行归一化。
 *
 * @param re 频域信号实部
 * @param im 频域信号虚部
 * @return QPair<时域实部, 时域虚部>
 */
QPair<QVector<double>, QVector<double>> RecursiveDFT2::inverse(const QVector<double>& re,
                                                                 const QVector<double>& im)
{
    QElapsedTimer timer;
    timer.start();

    /* 逆变换：取共轭 → 正变换 → 再取共轭 → 除以N */
    QVector<double> conjRe(m_n, 0.0);
    QVector<double> conjIm(m_n, 0.0);
    int len = qMin(m_n, qMin(re.size(), im.size()));
    for (int i = 0; i < len; ++i) {
        conjRe[i] = re[i];
        conjIm[i] = -im[i]; /* 取共轭 */
    }

    QVector<double> outRe(m_n, 0.0);
    QVector<double> outIm(m_n, 0.0);

    ditfft(conjRe, conjIm, outRe, outIm, m_n, 1);

    /* 再取共轭并归一化 */
    for (int i = 0; i < m_n; ++i) {
        outIm[i] = -outIm[i];
        outRe[i] /= m_n;
        outIm[i] /= m_n;
    }

    /* 更新统计 */
    m_stats.totalTransforms++;
    m_stats.totalPoints += m_n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalTransforms;

    emit transformCompleted(m_n);
    return {outRe, outIm};
}

/**
 * @brief 重置所有统计数据
 */
void RecursiveDFT2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
