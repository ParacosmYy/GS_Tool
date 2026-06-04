/**
 * @file FftEngine.h
 * @brief FFT频谱计算引擎 -- Cooley-Tukey radix-2 DIT算法实现
 *
 * 设计: 纯计算类(不依赖QWidget)、Cooley-Tukey radix-2 DIT算法、四种窗函数(矩形/汉宁/海明/布莱克曼)
 * 非二次幂长度自动零填充、返回单边幅度谱(频率→幅度)
 * 协作: FftWidget(调用compute()获取频谱数据) / ChartModel(通过FftWidget间接提供时域数据)
 */

#ifndef FFTENGINE_H
#define FFTENGINE_H

#include <QObject>
#include <QVector>
#include <QPointF>
#include <complex>

/// @brief FFT频谱计算引擎 — 封装Cooley-Tukey radix-2 FFT算法和常用窗函数，接收时域采样数据，输出频率-幅度谱
class FftEngine : public QObject {
    Q_OBJECT

public:
    /// @brief 窗函数类型 — Rectangular(最高分辨率/最差泄漏)、Hanning(均衡推荐)、Hamming(旁瓣衰减稍弱)、Blackman(最强抑制/最低分辨率)
    enum class WindowType {
        Rectangular,  ///< 矩形窗（无窗）
        Hanning,      ///< 汉宁窗
        Hamming,      ///< 海明窗
        Blackman      ///< 布莱克曼窗
    };
    Q_ENUM(WindowType)

    /** @brief 构造FFT引擎 @param parent 父对象 */
    explicit FftEngine(QObject* parent = nullptr);

    /** @brief 计算频谱 — 加窗→FFT→单边幅度谱，数据量不足fftSize则补零 @param timeData 时域采样点(x=时间/序号, y=采样值) @param sampleRate 采样率Hz @param window 窗函数类型 @param fftSize FFT运算长度(2的幂)，0=自动nextPowerOf2 @return 频谱点集(x=频率Hz, y=幅度)，长度fftSize/2 */
    QVector<QPointF> compute(const QVector<QPointF>& timeData,
                             double sampleRate,
                             WindowType window = WindowType::Hanning,
                             int fftSize = 0);

    /** @brief 计算大于等于n的最小2的幂 @param n 输入值 @return >= n 的最小2的幂 */
    static int nextPowerOf2(int n);

    /** @brief 将窗函数类型转为可读字符串 @param window 窗函数类型 @return 窗函数名称（英文） */
    static QString windowTypeName(WindowType window);

    // ---- 统计 getter ----

    /** @brief 获取总FFT变换执行次数 */
    quint64 totalTransforms() const;

    /** @brief 获取成功执行的FFT变换次数（不含错误） @return 成功执行计数 */
    quint64 totalTransformsExecuted() const { return m_totalTransformsExecuted; }

    /** @brief 获取总处理的采样点数（累计，跨所有变换） */
    quint64 totalSamplesProcessed() const;

    /** @brief 获取单次变换处理过的最大采样点数（峰值） */
    quint64 maxSampleSize() const;

    /** @brief 获取FFT计算中发生的错误次数（空数据/无效参数等） */
    quint64 errorCount() const;

    /** @brief 重置所有统计计数器为初始值 */
    void resetFftStatistics();

    /** @brief 重置所有统计计数器（别名，调用resetFftStatistics） */
    void resetStats();

signals:
    /** @brief 频谱计算完成信号 @param spectrum 频谱数据（频率→幅度） @param fundamentalFreq 基频Hz（最大峰值频率） */
    void spectrumComputed(const QVector<QPointF>& spectrum, double fundamentalFreq);

private:
    /** @brief 对数据序列应用窗函数 @param data 输入/输出数据序列（原地修改） @param window 窗函数类型 */
    void applyWindow(QVector<std::complex<double>>& data, WindowType window);

    /** @brief 执行Cooley-Tukey radix-2 DIT FFT（原地计算） @param data 输入/输出复数序列（长度必须为2的幂） */
    void fftRadix2(QVector<std::complex<double>>& data);

    /** @brief 计算单边幅度谱 @param fftResult FFT输出复数序列 @param sampleRate 采样率Hz @return 频率-幅度点集 */
    QVector<QPointF> magnitudeSpectrum(const QVector<std::complex<double>>& fftResult,
                                       double sampleRate);

    /** @brief 计算位反转索引（用于蝶形运算前的数据重排） @param index 原始索引 @param bits 索引位数(log2(N)) @return 位反转后的索引 */
    static int bitReverse(int index, int bits);

    /** @brief 计算以2为底的对数向上取整 @param n 输入值（必须为2的幂） @return log2(n) */
    static int log2Int(int n);

    // 统计计数器
    quint64 m_totalTransforms = 0;        ///< 总FFT变换执行次数
    quint64 m_totalTransformsExecuted = 0;///< 成功执行的FFT变换次数（不含错误）
    quint64 m_totalSamplesProcessed = 0;  ///< 总处理的采样点数（累计）
    quint64 m_maxSampleSize = 0;          ///< 单次变换处理过的最大采样点数（峰值）
    quint64 m_errorCount = 0;             ///< FFT计算中发生的错误次数
};

#endif // FFTENGINE_H
