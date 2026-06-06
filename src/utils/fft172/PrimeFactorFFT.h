/**
 * @file PrimeFactorFFT.h
 * @brief 素因子FFT(PFA算法N=N1*N2且gcd=1) — Prime Factor Algorithm FFT for Length N = N1*N2 where gcd(N1,N2)=1
 *
 * 功能: 实现素因子分解FFT，适用于N=N1*N2且gcd(N1,N2)=1的长度、
 *       无需旋转因子、模映射索引重排和二维DFT分解。
 *
 * 协作: ZoomFFT(缩放FFT) / WinogradFFT4(Winograd) / FftEngine(FFT引擎)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief 素因子FFT处理器
 */
class PrimeFactorFFT : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalTransforms = 0;      ///< 累计变换次数
        double avgProcessingTimeMs = 0.0; ///< 平均耗时(ms)
        int lastN = 0;                    ///< 最近变换长度
        int lastN1 = 0;                   ///< 最近N1
        int lastN2 = 0;                   ///< 最近N2
    };

    explicit PrimeFactorFFT(QObject *parent = nullptr);
    ~PrimeFactorFFT() override;

    /**
     * @brief 正向PFA-FFT
     * @param real 实部输入/输出
     * @param imag 虚部输入/输出
     * @return true=成功
     */
    bool transform(QVector<double>& real, QVector<double>& imag);

    /**
     * @brief 逆向PFA-FFT
     * @param real 实部输入/输出
     * @param imag 虚部输入/输出
     * @return true=成功
     */
    bool inverseTransform(QVector<double>& real, QVector<double>& imag);

    /** @brief 检查N是否可用(N=N1*N2, gcd(N1,N2)=1) */
    bool isValidLength(int n) const;

    /** @brief 分解N为互素因子 */
    bool factorize(int n, int& n1, int& n2) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void transformCompleted(int n, int n1, int n2);

private:
    /** @brief 模逆(扩展欧几里得) */
    static int modInverse(int a, int m);

    /** @brief 最大公约数 */
    static int gcd(int a, int b);

    /** @brief 短DFT(直接计算) */
    void shortDFT(QVector<double>& r, QVector<double>& i,
                  int n, int stride, bool inverse) const;

    /** @brief Ruritanian索引映射 */
    void indexMap(int n1, int n2,
                  QVector<int>& rowToLinear, QVector<int>& colToLinear) const;

    Stats m_stats;
    double m_timeSum = 0.0;
};
