/**
 * @file FftEngine.h
 * @brief FFT频谱计算引擎 -- Cooley-Tukey radix-2 DIT算法实现
 *
 * 设计要点:
 *   1. 纯计算类，不依赖QWidget，仅继承QObject以支持信号
 *   2. 实现 Cooley-Tukey radix-2 DIT（时间抽取）FFT算法
 *   3. 提供四种窗函数: 矩形窗、汉宁窗、海明窗、布莱克曼窗
 *   4. 输入长度非2的幂时自动零填充到最近的2^N
 *   5. 返回单边幅度谱（频率→幅度），便于频谱显示
 *
 * 协作关系:
 *   - FftWidget: 调用compute()获取频谱数据用于渲染
 *   - ChartModel: 通过FftWidget间接提供时域数据
 */

#ifndef FFTENGINE_H
#define FFTENGINE_H

#include <QObject>
#include <QVector>
#include <QPointF>
#include <complex>

/**
 * @brief FFT频谱计算引擎
 *
 * 封装 Cooley-Tukey radix-2 FFT算法和常用窗函数，
 * 接收时域采样数据，输出频率-幅度谱。
 * 可独立于任何QWidget使用。
 */
class FftEngine : public QObject {
    Q_OBJECT

public:
    /**
     * @brief 窗函数类型枚举
     *
     * 不同窗函数在频谱泄漏抑制能力和频率分辨率之间有不同权衡:
     *   - Rectangular: 最高频率分辨率，最差泄漏抑制
     *   - Hanning: 均衡之选，一般用途推荐
     *   - Hamming: 类似Hanning，旁瓣衰减稍弱但主瓣略窄
     *   - Blackman: 最强泄漏抑制，频率分辨率最低
     */
    enum class WindowType {
        Rectangular,  ///< 矩形窗（无窗）
        Hanning,      ///< 汉宁窗
        Hamming,      ///< 海明窗
        Blackman      ///< 布莱克曼窗
    };
    Q_ENUM(WindowType)

    /**
     * @brief 构造FFT引擎
     * @param parent 父对象
     */
    explicit FftEngine(QObject* parent = nullptr);

    /**
     * @brief 计算频谱
     *
     * 对输入时域数据执行: 加窗 → FFT → 单边幅度谱计算。
     * 若数据量不足 fftSize 则在末尾补零。
     *
     * @param timeData 时域采样点（x=时间/序号, y=采样值）
     * @param sampleRate 采样率（Hz），用于计算频率轴
     * @param window 窗函数类型
     * @param fftSize FFT运算长度（必须为2的幂），0表示自动取数据长度的nextPowerOf2
     * @return 频谱数据点集（x=频率Hz, y=幅度），长度为 fftSize/2
     */
    QVector<QPointF> compute(const QVector<QPointF>& timeData,
                             double sampleRate,
                             WindowType window = WindowType::Hanning,
                             int fftSize = 0);

    /**
     * @brief 计算大于等于n的最小2的幂
     * @param n 输入值
     * @return >= n 的最小2的幂
     */
    static int nextPowerOf2(int n);

    /**
     * @brief 将窗函数类型转为可读字符串
     * @param window 窗函数类型
     * @return 窗函数名称（英文）
     */
    static QString windowTypeName(WindowType window);

    // ---- 统计 getter ----

    /** @brief 获取总FFT变换执行次数 */
    quint64 totalTransforms() const;

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
    /**
     * @brief 频谱计算完成信号
     * @param spectrum 频谱数据（频率→幅度）
     * @param fundamentalFreq 基频（Hz），即频谱中最大峰值对应的频率
     */
    void spectrumComputed(const QVector<QPointF>& spectrum, double fundamentalFreq);

private:
    /**
     * @brief 对数据序列应用窗函数
     * @param data 输入/输出数据序列（原地修改）
     * @param window 窗函数类型
     */
    void applyWindow(QVector<std::complex<double>>& data, WindowType window);

    /**
     * @brief 执行 Cooley-Tukey radix-2 DIT FFT（原地计算）
     * @param data 输入/输出复数序列（长度必须为2的幂）
     */
    void fftRadix2(QVector<std::complex<double>>& data);

    /**
     * @brief 计算单边幅度谱
     * @param fftResult FFT输出复数序列
     * @param sampleRate 采样率（Hz）
     * @return 频率-幅度点集
     */
    QVector<QPointF> magnitudeSpectrum(const QVector<std::complex<double>>& fftResult,
                                       double sampleRate);

    /**
     * @brief 计算位反转索引（用于蝶形运算前的数据重排）
     * @param index 原始索引
     * @param bits 索引位数（log2(N)）
     * @return 位反转后的索引
     */
    static int bitReverse(int index, int bits);

    /**
     * @brief 计算以2为底的对数向上取整（即log2的整数部分）
     * @param n 输入值（必须为2的幂）
     * @return log2(n)
     */
    static int log2Int(int n);

    // 统计计数器
    quint64 m_totalTransforms = 0;        ///< 总FFT变换执行次数
    quint64 m_totalSamplesProcessed = 0;  ///< 总处理的采样点数（累计）
    quint64 m_maxSampleSize = 0;          ///< 单次变换处理过的最大采样点数（峰值）
    quint64 m_errorCount = 0;             ///< FFT计算中发生的错误次数
};

#endif // FFTENGINE_H
