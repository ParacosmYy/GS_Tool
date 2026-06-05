/**
 * @file MuLawEncoder.h
 * @brief μ-law/A-law压扩编解码器 — ITU-T G.711标准
 *
 * 提供μ-law(北美/日本)和A-law(欧洲)压扩(companding)算法,
 * 符合ITU-T G.711标准。将16位线性PCM压缩为8位非线性编码,
 * 适用于嵌入式调试中的音频数据处理和语音编解码。
 */
#ifndef MU_LAW_ENCODER_H
#define MU_LAW_ENCODER_H

#include <QObject>
#include <QByteArray>
#include <QVector>

/**
 * @class MuLawEncoder
 * @brief μ-law/A-law压扩编解码器
 *
 * 支持μ-law(μ=255)和A-law(A=87.6)两种压扩标准,
 * 提供16位PCM到8位压扩编码的双向转换。
 */
class MuLawEncoder : public QObject {
    Q_OBJECT

public:
    /** @brief 压扩标准 */
    enum Law {
        MuLaw = 0,      ///< μ-law(北美/日本标准)
        ALaw = 1         ///< A-law(欧洲标准)
    };
    Q_ENUM(Law)

    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalEncodes = 0;       ///< 编码操作总次数
        quint64 totalDecodes = 0;       ///< 解码操作总次数
        quint64 totalSamples = 0;       ///< 处理的采样总数
        double  avgSnr = 0.0;           ///< 平均信噪比(dB)
    };

    /** @brief 构造函数 @param parent 父对象 */
    explicit MuLawEncoder(QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~MuLawEncoder() override;

    // ── 配置 ──

    /** @brief 设置压扩标准 */
    void setLaw(Law law);

    /** @brief 获取当前压扩标准 */
    Law law() const;

    // ── 编解码(采样级) ──

    /**
     * @brief 压扩编码: 16位PCM → 8位
     * @param sample 16位有符号PCM采样
     * @return 8位压扩编码
     */
    quint8 encodeSample(qint16 sample);

    /**
     * @brief 压扩解码: 8位 → 16位PCM
     * @param code 8位压扩编码
     * @return 16位有符号PCM采样
     */
    qint16 decodeSample(quint8 code);

    // ── 编解码(批量) ──

    /**
     * @brief 批量编码PCM字节数组
     * @param pcmData 16位PCM数据(小端序, 2字节/采样)
     * @return 8位压扩编码数据
     */
    QByteArray encode(const QByteArray& pcmData);

    /**
     * @brief 批量解码压扩编码
     * @param compandedData 8位压扩数据
     * @return 16位PCM数据(小端序)
     */
    QByteArray decode(const QByteArray& compandedData);

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 @param count 采样数 @param ratio 压缩比 */
    void encoded(int count, double ratio);
    /** @brief 解码完成信号 @param count 采样数 */
    void decoded(int count);
    /** @brief 错误信号 @param msg 错误描述 */
    void error(const QString& msg);

private:
    /** @brief μ-law编码查表 */
    static quint8 muLawEncode(qint16 sample);

    /** @brief μ-law解码查表 */
    static qint16 muLawDecode(quint8 code);

    /** @brief A-law编码查表 */
    static quint8 aLawEncode(qint16 sample);

    /** @brief A-law解码查表 */
    static qint16 aLawDecode(quint8 code);

    /** @brief 计算信噪比 */
    double computeSnr(qint16 original, qint16 reconstructed);

    Law m_law = MuLaw;               ///< 当前压扩标准
    mutable Stats m_stats;           ///< 操作统计
};

#endif // MU_LAW_ENCODER_H
