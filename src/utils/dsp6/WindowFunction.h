/**
 * @file WindowFunction.h
 * @brief DSP窗函数 — Kaiser/Chebyshev/Gaussian/Dolph-Chebyshev
 *
 * 功能: 提供多种高级窗函数用于频谱分析和FIR滤波器设计,
 *       支持参数化窗函数生成和窗函数特性计算。
 *
 * 协作: SpectrumAnalyzer(频谱分析) / DigitalFilter(滤波器设计)
 */

#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief DSP窗函数生成器
 *
 * 支持的窗函数类型:
 * - Kaiser: 可调主瓣宽度和旁瓣衰减
 * - Chebyshev: 等纹波旁瓣, 最窄主瓣
 * - Gaussian: 高斯形状, 可调标准差
 * - Dolph-Chebyshev: 最优Dolph-Chebyshev窗
 */
class WindowFunction : public QObject
{
    Q_OBJECT

public:
    /** @brief 窗函数类型 */
    enum class Type {
        Kaiser,             ///< Kaiser窗 (可调β参数)
        Chebyshev,          ///< Chebyshev窗 (等纹波旁瓣)
        Gaussian,           ///< Gaussian窗 (可调σ参数)
        DolphChebyshev,     ///< Dolph-Chebyshev窗 (最优DPSS)
        Hamming,            ///< Hamming窗
        Blackman,           ///< Blackman窗
        FlatTop             ///< Flat-top窗 (测量用)
    };
    Q_ENUM(Type)

    /** @brief 窗函数特性 */
    struct WindowProperties {
        double coherentGain = 0.0;          ///< 相干增益
        double equivalentNoiseBW = 0.0;     ///< 等效噪声带宽(归一化)
        double scallopingLoss = 0.0;        ///< 扇形损耗(dB)
        double worstCaseProcessGain = 0.0;  ///< 最差处理增益(dB)
        double sideLobeLevel = 0.0;         ///< 最高旁瓣电平(dB)
        double sideLobeFallOff = 0.0;       ///< 旁瓣衰减率(dB/oct)
        double mainLobeWidth3dB = 0.0;      ///< 3dB主瓣宽度(归一化)
    };

    /** @brief 统计信息 */
    struct Stats {
        int totalWindowsGenerated = 0;      ///< 累计生成窗数
        int totalPointsComputed = 0;        ///< 累计计算点数
        int totalPropertiesComputed = 0;    ///< 累计特性计算次数
        double avgProcessingTimeMs = 0.0;   ///< 平均处理耗时(ms)
    };

    explicit WindowFunction(QObject* parent = nullptr);

    /**
     * @brief 生成Kaiser窗
     * @param N 窗长度
     * @param beta 形状参数(旁瓣衰减控制, 典型值5~9)
     * @return 窗系数数组
     */
    QVector<double> kaiser(int N, double beta = 8.0);

    /**
     * @brief 生成Chebyshev窗(等纹波)
     * @param N 窗长度
     * @param attenuation 旁瓣衰减(dB, >0)
     * @return 窗系数数组
     */
    QVector<double> chebyshev(int N, double attenuation = 60.0);

    /**
     * @brief 生成Gaussian窗
     * @param N 窗长度
     * @param sigma 标准差(N归一化, 典型值0.3~0.5)
     * @return 窗系数数组
     */
    QVector<double> gaussian(int N, double sigma = 0.4);

    /**
     * @brief 生成Dolph-Chebyshev窗
     * @param N 窗长度
     * @param attenuation 旁瓣衰减(dB, >0)
     * @return 窗系数数组
     */
    QVector<double> dolphChebyshev(int N, double attenuation = 60.0);

    /**
     * @brief 生成指定类型的窗函数
     * @param type 窗函数类型
     * @param N 窗长度
     * @param param 可选参数(beta/sigma/attenuation)
     * @return 窗系数数组
     */
    QVector<double> generate(Type type, int N, double param = 0.0);

    /**
     * @brief 计算窗函数特性
     * @param window 窗系数数组
     * @return 窗函数特性
     */
    WindowProperties computeProperties(const QVector<double>& window);

    /**
     * @brief 将窗函数应用到数据
     * @param data 输入数据
     * @param window 窗系数
     * @return 加窗后的数据
     */
    QVector<double> apply(const QVector<double>& data,
                          const QVector<double>& window);

    Stats stats() const;
    void resetStatistics();

signals:
    /** @brief 窗函数生成完成 @param type 类型 @param length 长度 */
    void windowGenerated(Type type, int length);

private:
    /**
     * @brief 零阶修正Bessel函数 I0(x)
     * @param x 参数
     * @return I0(x)值
     */
    double besselI0(double x) const;

    /**
     * @brief Chebyshev多项式 T_n(x)
     * @param n 阶数
     * @param x 参数
     * @return T_n(x)值
     */
    double chebyshevPoly(int n, double x) const;

    /**
     * @brief 计算DFT幅度谱(用于特性分析)
     * @param window 窗系数
     * @param fftSize FFT点数
     * @return 幅度谱
     */
    QVector<double> windowSpectrum(const QVector<double>& window, int fftSize);

    Stats m_stats;
    double m_timeSum = 0.0;
};
