#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Reed-Solomon纠错码实现 (GF(2^11)域)
 *
 * 提供基于有限域算术的RS编码/解码，支持突发错误纠正和擦除恢复。
 */
class ReedSolomon11 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构
    struct Stats {
        int totalEncodingRuns = 0;      ///< 总编码次数
        double avgProcessingTimeMs = 0.0; ///< 平均处理耗时(ms)
        int totalErrorsCorrected = 0;   ///< 总纠正错误数
    };

    explicit ReedSolomon11(QObject* parent = nullptr);
    Stats stats() const { return m_stats; }
    void resetStatistics();

    /**
     * @brief RS编码
     * @param message 信息符号序列
     * @param parityCount 校验符号数量
     * @return 编码后的完整码字
     */
    QVector<int> encode(const QVector<int>& message, int parityCount);

    /**
     * @brief RS译码（纠正错误和擦除）
     * @param received 接收码字（含错误）
     * @param erasurePositions 已知擦除位置
     * @return 译码后的信息符号
     */
    QVector<int> decode(const QVector<int>& received, const QVector<int>& erasurePositions = {});

    /**
     * @brief 获取当前纠错能力
     * @return 可纠正的符号错误数
     */
    int errorCorrectionCapacity() const { return m_parityCount / 2; }

    /**
     * @brief 设置本原多项式
     * @param polynomial 本原多项式系数
     */
    void setPrimitivePolynomial(int polynomial);

signals:
    /// 编码完成信号
    void encodingCompleted(int codewordLength);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_parityCount = 0;
    int m_primitivePoly = 0;
};
