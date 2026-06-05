/**
 * @file LdpcEncoder.h
 * @brief LDPC系统编码器 — 校验矩阵构造/Richardson-Urbanke编码
 *
 * 功能: 构造LDPC校验矩阵(PEG/规则构造)，实现近似下三角形式
 *       (Approximate Lower Triangular, ALT)的Richardson-Urbanke编码，
 *       支持系统编码和伴随式校验。
 *
 * 协作: DataCompressor(数据压缩) / CRC(差错检测)
 */
#ifndef LDPCENCODER_H
#define LDPCENCODER_H

#include <QObject>
#include <QVector>
#include <QList>
#include <QPair>

/**
 * @brief LDPC系统编码器
 */
class LdpcEncoder : public QObject {
    Q_OBJECT

public:
    /** @brief 矩阵构造方法 */
    enum class ConstructionMethod {
        Regular,    ///< 规则构造(列重/行重固定)
        PEG         ///< Progressive Edge Growth
    };
    Q_ENUM(ConstructionMethod)

    /** @brief 编码参数 */
    struct LdpcParams {
        int blockLength = 0;        ///< 码长N
        int messageLength = 0;      ///< 信息位K
        int columnWeight = 3;       ///< 列重(校验矩阵)
        int rowWeight = 6;          ///< 行重(校验矩阵)
    };

    /** @brief 统计 */
    struct Stats {
        quint64 totalEncodings = 0;         ///< 累计编码次数
        quint64 totalBitsEncoded = 0;       ///< 累计编码比特数
        double  avgProcessingTimeMs = 0.0;  ///< 平均处理时间(ms)
        quint64 totalSyndromeChecks = 0;    ///< 累计伴随式校验次数
    };

    explicit LdpcEncoder(QObject* parent = nullptr);

    /** @brief 初始化编码器 @param params LDPC参数 @param method 构造方法 @return 是否成功 */
    bool initialize(const LdpcParams& params, ConstructionMethod method = ConstructionMethod::PEG);

    /** @brief 系统编码 @param message 信息比特(K位) @return 码字(N位) */
    QVector<int> encode(const QVector<int>& message);

    /** @brief 批量编码 @param messages 信息比特列表 @return 码字列表 */
    QList<QVector<int>> encodeBatch(const QList<QVector<int>>& messages);

    /** @brief 伴随式校验 @param codeword 码字 @return 是否通过(无错误) */
    bool syndromeCheck(const QVector<int>& codeword) const;

    /** @brief 获取校验矩阵 @return 稀疏表示(行->[(列,值)]) */
    QList<QList<QPair<int, int>>> parityCheckMatrix() const;

    /** @brief 获取当前参数 @return LDPC参数 */
    LdpcParams params() const { return m_params; }

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param blockLength 码字长度 @param numEncoded 编码块数 */
    void encodingComplete(int blockLength, int numEncoded);

    /** @brief 矩阵构造完成 @param rows 行数 @param cols 列数 */
    void matrixConstructed(int rows, int cols);

private:
    void constructRegular();
    void constructPEG();
    void buildGeneratorMatrix();
    QVector<int> richardsonUrbankeEncode(const QVector<int>& msg) const;
    QVector<int> matrixVectorMultiply(
        const QList<QList<QPair<int, int>>>& matrix,
        const QVector<int>& vec) const;

    LdpcParams m_params;            ///< 编码参数
    ConstructionMethod m_method;    ///< 构造方法

    /* 校验矩阵H: 稀疏表示, 每行存储(列索引,值)对 */
    QList<QList<QPair<int, int>>> m_H;

    /* ALT分解相关的子矩阵 */
    QList<QList<QPair<int, int>>> m_A; ///< ALT的A子矩阵
    QList<QList<QPair<int, int>>> m_B; ///< ALT的B子矩阵
    QList<QList<QPair<int, int>>> m_C; ///< ALT的C子矩阵
    QList<QList<QPair<int, int>>> m_D; ///< ALT的D子矩阵
    QList<QList<QPair<int, int>>> m_E; ///< ALT的E子矩阵
    QList<QList<QPair<int, int>>> m_T; ///< ALT的T子矩阵(下三角)
    int m_gap = 0;                      ///< ALT的间隙参数g

    bool m_initialized = false;     ///< 是否已初始化

    Stats m_stats;
    double m_timeSum = 0.0;         ///< 处理时间累加器
};

#endif // LDPCENCODER_H
