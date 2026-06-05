#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief BCH纠错码实现 (10位参数配置)
 *
 * 提供BCH编码和解码，支持可配置的纠错能力t和码长参数。
 */
class BchCode10 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalEncodingRuns = 0;      ///< 总编码次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int totalErrorsCorrected = 0;   ///< 总纠正错误数
    };

    explicit BchCode10(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief 生成BCH码参数
     * @param m 伽罗瓦域阶数参数 (GF(2^m))
     * @param t 纠错能力
     * @return 参数生成是否成功
     */
    bool generateParameters(int m, int t);

    /**
     * @brief BCH编码
     * @param message 信息比特序列
     * @return 编码后的码字
     */
    QVector<int> encode(const QVector<int>& message);

    /**
     * @brief BCH译码（Berlekamp-Massey算法）
     * @param received 接收码字
     * @return 译码后的信息比特
     */
    QVector<int> decode(const QVector<int>& received);

    /**
     * @brief 获取当前码参数
     * @return [n, k, t] 码长、信息长度、纠错能力
     */
    QVector<int> codeParameters() const;

signals:
    /// 编码完成信号
    void encodingCompleted(int codewordLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_n = 0;  ///< 码长
    int m_k = 0;  ///< 信息长度
    int m_t = 0;  ///< 纠错能力
    int m_m = 0;  ///< 域阶数
};
