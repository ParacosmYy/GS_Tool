/**
 * @file RsErasure.h
 * @brief Reed-Solomon纠删码引擎 — Cauchy RS编码/Vandermonde矩阵/高斯消元解码
 *
 * 功能: 实现Reed-Solomon纠删码的编码与解码，支持Cauchy矩阵和
 *       Vandermonde矩阵两种编码方式，通过高斯消元实现数据恢复。
 *
 * 协作: DataCompressor(数据压缩) / StreamCaptureRecorder(流录制)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @brief Reed-Solomon纠删码引擎 — 数据冗余编码与擦除恢复
 */
class RsErasure : public QObject {
    Q_OBJECT

public:
    /** @brief 编码矩阵类型 */
    enum class MatrixType {
        Vandermonde,    ///< Vandermonde矩阵编码
        Cauchy          ///< Cauchy矩阵编码
    };
    Q_ENUM(MatrixType)

    /** @brief 统计 */
    struct Stats {
        quint64 totalEncodes = 0;               ///< 累计编码次数
        quint64 totalDecodes = 0;               ///< 累计解码次数
        quint64 totalShardsProcessed = 0;       ///< 累计处理分片数
        quint64 totalDecodesFailed = 0;         ///< 累计解码失败次数
        double  avgProcessingTimeMs = 0.0;      ///< 平均处理时间(ms)
    };

    explicit RsErasure(QObject* parent = nullptr);

    /** @brief 初始化编解码参数 @param dataShards 数据分片数 @param parityShards 校验分片数 @param type 矩阵类型 @return 是否成功 */
    bool initialize(int dataShards, int parityShards,
                    MatrixType type = MatrixType::Cauchy);

    /** @brief 编码数据 @param data 原始数据 @return 编码后的分片列表(数据+校验) */
    QVector<QByteArray> encode(const QByteArray& data);

    /** @brief 解码恢复数据 @param shards 分片列表(缺失位置为空QByteArray) @return 恢复的原始数据 */
    QByteArray decode(const QVector<QByteArray>& shards);

    /** @brief 获取数据分片数 @return 数据分片数 */
    int dataShards() const { return m_dataShards; }

    /** @brief 获取校验分片数 @return 校验分片数 */
    int parityShards() const { return m_parityShards; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param totalShards 总分片数 @param dataSize 原始数据大小 */
    void encodeComplete(int totalShards, int dataSize);

    /** @brief 解码完成 @param recoveredSize 恢复数据大小 @param erasedCount 擦除数 */
    void decodeComplete(int recoveredSize, int erasedCount);

    /** @brief 解码失败 @param reason 失败原因 */
    void decodeFailed(const QString& reason);

private:
    void buildVandermondeMatrix();
    void buildCauchyMatrix();
    void gaussianElimination(QVector<quint8>& mat, int rows, int cols,
                             QVector<int>& pivotCols) const;
    quint8 gfMultiply(quint8 a, quint8 b) const;
    quint8 gfInverse(quint8 a) const;
    quint8 gfDivide(quint8 a, quint8 b) const;
    static quint8 gfAdd(quint8 a, quint8 b) { return a ^ b; }

    /** @brief GF(256)对数/反对数查找表 */
    void buildGfTables();

    int m_dataShards;           ///< 数据分片数
    int m_parityShards;        ///< 校验分片数
    MatrixType m_matrixType;   ///< 矩阵类型
    bool m_initialized;         ///< 是否已初始化

    QVector<quint8> m_encodeMatrix;   ///< 编码矩阵
    quint8 m_gfExp[512];             ///< GF指数表
    quint8 m_gfLog[256];             ///< GF对数表

    Stats m_stats;
    double m_timeSum = 0.0;    ///< 处理时间累加器
};
