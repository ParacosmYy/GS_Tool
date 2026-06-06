/**
 * @file ErasureCode4.h
 * @brief Reed-Solomon纠删码(Vandermonde矩阵编码+高斯消元解码) — Reed-Solomon Erasure Code with Vandermonde Matrix Encoding and Gaussian Elimination Decoding
 *
 * 功能: 实现Reed-Solomon纠删码编解码，使用Vandermonde矩阵生成编码分片，
 *       高斯消元法恢复丢失数据。适用于分布式存储和数据冗余。
 *
 * 协作: CrcCalculator(校验) / DataLogger(日志) / DataExporter(导出)
 */
#pragma once

#include <QObject>
#include <QVector>

/**
 * @brief Reed-Solomon纠删编解码器
 */
class ErasureCode4 : public QObject {
    Q_OBJECT

public:
    /** @brief 运行统计 */
    struct Stats {
        quint64 totalEncodes = 0;          ///< 累计编码次数
        quint64 totalDecodes = 0;          ///< 累计解码次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
        int lastDataShards = 0;            ///< 最近一次数据分片数
        int lastParityShards = 0;          ///< 最近一次校验分片数
    };

    explicit ErasureCode4(QObject* parent = nullptr);
    ~ErasureCode4() override;

    /**
     * @brief 初始化编解码参数
     * @param dataShards 数据分片数
     * @param parityShards 校验分片数
     */
    void init(int dataShards, int parityShards);

    /**
     * @brief 编码：从数据分片生成校验分片
     * @param data 数据分片(每片等长QVector<quint8>)
     * @return 校验分片
     */
    QVector<QVector<quint8>> encode(const QVector<QVector<quint8>>& data);

    /**
     * @brief 解码：从幸存分片恢复丢失数据
     * @param shards 所有分片(丢失位置为空QVector)
     * @param missing 丢失分片索引列表
     * @return 恢复后的完整数据分片
     */
    QVector<QVector<quint8>> decode(const QVector<QVector<quint8>>& shards,
                                    const QVector<int>& missing);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param totalShards 总分片数 */
    void encodeCompleted(int totalShards);
    /** @brief 解码完成 @param recovered 恢复分片数 */
    void decodeCompleted(int recovered);

private:
    /** @brief GF(256)有限域乘法 */
    static quint8 gfMul(quint8 a, quint8 b);

    /** @brief GF(256)有限域除法 */
    static quint8 gfDiv(quint8 a, quint8 b);

    /** @brief GF(256)有限域求逆 */
    static quint8 gfInv(quint8 a);

    /** @brief 构建Vandermonde编码矩阵 */
    void buildMatrix();

    /** @brief GF(256)对数表查找 */
    static int gfLog(quint8 a);

    /** @brief GF(256)反对数表查找 */
    static quint8 gfExp(int e);

    static QVector<quint8> s_expTable;
    static QVector<quint8> s_logTable;
    static bool s_tablesInit;

    /** @brief 初始化GF(256)对数/反对数表 */
    static void initGfTables();

    int m_dataShards = 0;
    int m_parityShards = 0;
    QVector<QVector<quint8>> m_matrix;

    Stats m_stats;
    double m_timeSum = 0.0;
};
