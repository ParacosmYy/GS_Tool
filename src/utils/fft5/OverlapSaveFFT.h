/**
 * @file OverlapSaveFFT.h
 * @brief Overlap-Save快速卷积引擎 — 基于FFT的分块线性卷积
 *
 * 利用FFT实现Overlap-Save算法对长信号与卷积核进行快速线性卷积,
 * 适用于实时数据流滤波、FIR滤波器加速等嵌入式信号处理场景。
 */
#ifndef OVERLAPSAVEFFT_H
#define OVERLAPSAVEFFT_H

#include <QObject>
#include <QVector>
#include <complex>
#include <vector>

/**
 * @class OverlapSaveFFT
 * @brief Overlap-Save快速卷积 — 分块FFT线性卷积
 *
 * 典型用法:
 * @code
 *   OverlapSaveFFT engine;
 *   engine.setBlockSize(256);
 *   auto output = engine.process(signal, kernel);
 * @endcode
 */
class OverlapSaveFFT : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalProcessings = 0;    ///< 总处理次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit OverlapSaveFFT(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~OverlapSaveFFT() override;

    // ── 核心接口 ──

    /**
     * @brief 执行Overlap-Save快速卷积
     * @param signal 输入信号
     * @param kernel 卷积核(较短序列)
     * @return 卷积结果(长度=signal.size())
     */
    QVector<double> process(const QVector<double>& signal,
                            const QVector<double>& kernel);

    /**
     * @brief 设置FFT块大小
     * @param size 块大小(自动调整为>=64的2的幂)
     */
    void setBlockSize(int size);

    /**
     * @brief 获取当前块大小
     * @return 块大小
     */
    int blockSize() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置所有统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 卷积处理完成信号 @param outputSize 输出长度 */
    void processingCompleted(int outputSize);

private:
    /** @brief 执行FFT(原地, Cooley-Tukey) */
    void fft(std::vector<std::complex<double>>& data);

    /** @brief 执行逆FFT(原地) */
    void ifft(std::vector<std::complex<double>>& data);

    /** @brief 将n补齐为2的幂 */
    static int nextPowerOf2(int n);

    /** @brief FFT块大小 */
    int m_blockSize = 256;

    /** @brief 操作统计 */
    Stats m_stats;
};

#endif // OVERLAPSAVEFFT_H
