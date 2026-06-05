/**
 * @file BlockInterleaver.h
 * @brief 块交织器 — 通信信道交织/解交织
 *
 * 功能: 对数据进行块交织/解交织，支持行列交织/对角交织，
 *       统计处理次数/块数/耗时。
 */
#ifndef BLOCKINTERLEAVER_H
#define BLOCKINTERLEAVER_H

#include <QObject>
#include <QByteArray>
#include <QVector>

class BlockInterleaver : public QObject {
    Q_OBJECT
public:
    enum class Mode { RowColumn, Diagonal, Helical };

    struct Stats {
        quint64 totalOperations = 0;
        quint64 totalBlocksProcessed = 0;
        double  averageProcessingTimeMs = 0.0;
    };

    explicit BlockInterleaver(QObject* parent = nullptr);

    /** @brief 设置交织深度 @param rows 行数 @param cols 列数 */
    void setDimensions(int rows, int cols);

    /** @brief 设置交织模式 */
    void setMode(Mode mode);

    /** @brief 交织 @param data 输入 @return 交织后数据 */
    QByteArray interleave(const QByteArray& data);

    /** @brief 解交织 @param data 交织数据 @return 原始数据 */
    QByteArray deinterleave(const QByteArray& data);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(int blockSize);

private:
    QByteArray rowColumnOp(const QByteArray& data, bool inverse);
    QByteArray diagonalOp(const QByteArray& data, bool inverse);

    int m_rows;
    int m_cols;
    Mode m_mode;
    Stats m_stats;
    double m_timeSum;
};

#endif // BLOCKINTERLEAVER_H
