#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief 格雷码(Gray Code)编解码器实现
 *
 * 生成和转换二进制格雷码序列，相邻两个码字之间只有一位不同，
 * 适用于旋转编码器、卡诺图化简和模拟-数字转换中的毛刺消除。
 */
class GrayCode9 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalConverted = 0; double avgProcessingTimeMs = 0.0; };

    explicit GrayCode9(QObject* parent = nullptr);

    /** @brief 设置格雷码位宽，决定生成序列的长度(2^n) */
    void setBitWidth(int bits);

    /** @brief 生成指定位宽的完整格雷码序列 */
    QVector<int> generateSequence();

    /** @brief 将自然二进制数转换为格雷码 */
    int binaryToGray(int binary);

    /** @brief 将格雷码转换回自然二进制数 */
    int grayToBinary(int gray);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 转换完成信号，返回处理的码字数 */
    void conversionCompleted(int codeCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_bitWidth = 8;
};
