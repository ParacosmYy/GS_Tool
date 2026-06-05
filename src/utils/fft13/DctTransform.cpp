/**
 * @file DctTransformV2.cpp
 * @brief 离散余弦变换实现 — DCT-I/II/III/IV 四种类型
 */

#include "utils/fft13/DctTransform.h"

#include <QElapsedTimer>
#include <QtMath>

/** @brief 构造函数 @param parent 父对象 */
DctTransformV2::DctTransformV2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 执行正向 DCT 变换
 * @param input 输入时域数据
 * @param type DCT 类型
 * @return 变换结果
 */
QVector<double> DctTransformV2::forward(const QVector<double>& input, DctType type)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    int n = input.size();
    if (n < 2) {
        return input;
    }

    switch (type) {
    case DctType::TypeI:
        result = dctTypeI(input);
        break;
    case DctType::TypeII:
        result = dctTypeII(input);
        break;
    case DctType::TypeIII:
        result = dctTypeIII(input);
        break;
    case DctType::TypeIV:
        result = dctTypeIV(input);
        break;
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalTransforms;
    m_stats.totalPointsProcessed += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(n, type);
    return result;
}

/**
 * @brief 执行逆向 DCT 变换
 * @param coefficients DCT 系数
 * @param type DCT 类型
 * @return 时域数据
 */
QVector<double> DctTransformV2::inverse(const QVector<double>& coefficients,
                                      DctType type)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    /* DCT-I 的逆是自身 (除以 N-1) */
    /* DCT-II 的逆是 DCT-III (除以缩放) */
    switch (type) {
    case DctType::TypeI:
        result = dctTypeI(coefficients);
        /* DCT-I 逆变换需要除以 (N-1) */
        for (auto& v : result) v /= (coefficients.size() - 1);
        break;
    case DctType::TypeII:
        result = dctTypeIII(coefficients);
        break;
    case DctType::TypeIII:
        result = dctTypeII(coefficients);
        break;
    case DctType::TypeIV:
        /* DCT-IV 的逆是自身 */
        result = dctTypeIV(coefficients);
        for (auto& v : result) v /= coefficients.size();
        break;
    }

    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalTransforms;
    m_stats.totalPointsProcessed += coefficients.size();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalTransforms);

    emit transformCompleted(coefficients.size(), type);
    return result;
}

/**
 * @brief 截断高频系数
 * @param coefficients DCT 系数
 * @param keepCount 保留个数
 * @return 截断后的系数
 */
QVector<double> DctTransformV2::truncateCoefficients(
    const QVector<double>& coefficients, int keepCount) const
{
    int n = coefficients.size();
    int keep = qBound(1, keepCount, n);
    QVector<double> truncated(keep, 0.0);
    for (int i = 0; i < keep; ++i) {
        truncated[i] = coefficients[i];
    }
    return truncated;
}

/** @brief 重置统计信息 */
void DctTransformV2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief DCT-I 变换
 * X[k] = sum_{n=0}^{N-1} x[n] * cos(pi*n*k/(N-1))
 */
QVector<double> DctTransformV2::dctTypeI(const QVector<double>& input)
{
    int n = input.size();
    if (n < 2) return input;

    int fftSize = 2 * (n - 1);
    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);

    /* 构造对称扩展序列 */
    for (int i = 0; i < n; ++i) {
        real[i] = input[i];
        if (i > 0 && i < n - 1) {
            real[fftSize - i] = input[i];
        }
    }

    fft(real, imag);

    QVector<double> result(n);
    for (int k = 0; k < n; ++k) {
        result[k] = real[k];
    }
    return result;
}

/**
 * @brief DCT-II 变换 (标准压缩DCT)
 * X[k] = sum_{n=0}^{N-1} x[n] * cos(pi*(2n+1)*k/(2N))
 */
QVector<double> DctTransformV2::dctTypeII(const QVector<double>& input)
{
    int n = input.size();

    /* 补零到 2 的幂 */
    int fftSize = 1;
    while (fftSize < 2 * n) fftSize *= 2;

    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);

    /* 预旋转: x[n] * exp(-j*pi*n/(2N)) 并填入 FFT */
    for (int i = 0; i < n; ++i) {
        double angle = -M_PI * i / (2.0 * n);
        real[i] = input[i] * qCos(angle);
        imag[i] = input[i] * qSin(angle);
    }

    fft(real, imag);

    /* 后旋转 */
    QVector<double> result(n);
    for (int k = 0; k < n; ++k) {
        double angle = -M_PI * k / (2.0 * n);
        double cosA = qCos(angle);
        double sinA = qSin(angle);
        /* Re(Z_k * e^{-j*pi*k/(2N)}) */
        result[k] = 2.0 * (real[k] * cosA - imag[k] * sinA);
    }

    ++m_stats.totalFFTCalls;
    return result;
}

/**
 * @brief DCT-III 变换 (DCT-II 的逆)
 * X[k] = x[0]/2 + sum_{n=1}^{N-1} x[n] * cos(pi*n*(2k+1)/(2N))
 */
QVector<double> DctTransformV2::dctTypeIII(const QVector<double>& input)
{
    int n = input.size();

    /* 构造 FFT 输入: 预旋转 */
    int fftSize = 1;
    while (fftSize < 2 * n) fftSize *= 2;

    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);

    for (int k = 0; k < n; ++k) {
        double angle = -M_PI * k / (2.0 * n);
        real[k] = input[k] * qCos(angle);
        imag[k] = input[k] * qSin(angle);
    }

    fft(real, imag);

    QVector<double> result(n);
    for (int i = 0; i < n; ++i) {
        double angle = M_PI * i / (2.0 * n);
        result[i] = 2.0 * (real[i] * qCos(angle) - imag[i] * qSin(angle));
    }

    /* 缩放: 除以 2N */
    for (auto& v : result) v /= (2.0 * n);

    ++m_stats.totalFFTCalls;
    return result;
}

/**
 * @brief DCT-IV 变换
 * X[k] = sum_{n=0}^{N-1} x[n] * cos(pi*(2n+1)*(2k+1)/(4N))
 */
QVector<double> DctTransformV2::dctTypeIV(const QVector<double>& input)
{
    int n = input.size();

    /* DCT-IV = DCT-II of shifted input with post-shift */
    /* 使用 4N 点 FFT 实现 */
    int fftSize = 1;
    while (fftSize < 4 * n) fftSize *= 2;

    QVector<double> real(fftSize, 0.0), imag(fftSize, 0.0);

    /* 填充: x[n] 位于位置 2n+1，其余为 0 */
    for (int i = 0; i < n; ++i) {
        real[2 * i + 1] = input[i];
    }

    fft(real, imag);

    QVector<double> result(n);
    for (int k = 0; k < n; ++k) {
        result[k] = real[2 * k + 1] * 2.0;
    }

    ++m_stats.totalFFTCalls;
    return result;
}

/**
 * @brief 基 2 FFT (就地)
 * @param real 实部数组
 * @param imag 虚部数组
 */
void DctTransformV2::fft(QVector<double>& real, QVector<double>& imag)
{
    int n = real.size();
    if (n <= 1) return;
    imag.resize(n);
    imag.fill(0.0);

    /* 位反转排列 */
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
    }

    /* 蝶形运算 */
    for (int len = 2; len <= n; len *= 2) {
        double angle = -2.0 * M_PI / len;
        double wRe = qCos(angle), wIm = qSin(angle);

        for (int i = 0; i < n; i += len) {
            double curRe = 1.0, curIm = 0.0;
            for (int j = 0; j < len / 2; ++j) {
                int even = i + j;
                int odd = i + j + len / 2;

                double tRe = curRe * real[odd] - curIm * imag[odd];
                double tIm = curRe * imag[odd] + curIm * real[odd];

                real[odd] = real[even] - tRe;
                imag[odd] = imag[even] - tIm;
                real[even] += tRe;
                imag[even] += tIm;

                double newRe = curRe * wRe - curIm * wIm;
                double newIm = curRe * wIm + curIm * wRe;
                curRe = newRe;
                curIm = newIm;
            }
        }
    }
}
