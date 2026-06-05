#pragma once
#include <QObject>
#include <QVector>
#include <QPair>

/**
 * @brief Spinal码(脊码)编解码器实现
 *
 * 基于哈希函数的无限率编码方案，通过将输入比特分段映射到高维空间，
 * 利用连续编码和树搜索实现接近信道容量的可靠传输。
 */
class SpinalCode11 : public QObject {
    Q_OBJECT
public:
    /// 统计信息结构体
    struct Stats { int totalEncoded = 0; double avgProcessingTimeMs = 0.0; };

    explicit SpinalCode11(QObject* parent = nullptr);

    /** @brief 设置脊码的分段长度k(每次哈希输入的比特数) */
    void setSpineLength(int k);

    /** @brief 设置解码搜索树宽度，影响解码精度和复杂度 */
    void setSearchWidth(int width);

    /** @brief 对输入比特流执行Spinal编码，输出符号序列 */
    QVector<int> encode(const QVector<int>& bits);

    /** @brief 对接收符号序列执行Spinal解码，还原比特流 */
    QVector<int> decode(const QVector<double>& symbols, int messageLength);

    Stats stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成信号，返回输出符号数 */
    void encodingCompleted(int symbolCount);

private:
    Stats m_stats;
    double m_timeSum = 0.0;
    int m_spineK = 4;
    int m_searchWidth = 16;
};
