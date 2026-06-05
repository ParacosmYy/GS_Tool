/**
 * @file ReedSolomonCodec.h
 * @brief Reed-Solomon编解码器 — GF(2^8)纠错码
 *
 * 功能: GF(2^8)有限域算术，编码添加校验符号，
 *       Berlekamp-Massey解码，纠正t个符号错误，
 *       统计编解码次数/纠正数/耗时。
 */
#ifndef REEDSOLOMONCODEC_H
#define REEDSOLOMONCODEC_H

#include <QObject>
#include <QVector>

/**
 * @brief Reed-Solomon编解码器(GF(2^8))
 */
class ReedSolomonCodec : public QObject {
    Q_OBJECT

public:
    /** @brief 统计 */
    struct Stats {
        quint64 totalEncodes = 0;       ///< 累计编码次数
        quint64 totalDecodes = 0;       ///< 累计解码次数
        quint64 totalCorrections = 0;   ///< 累计纠正符号数
        quint64 totalFailures = 0;      ///< 累计解码失败次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间
    };

    /**
     * @brief 构造函数
     * @param nsym 校验符号数(纠错能力=nsym/2)
     * @param parent 父对象
     */
    explicit ReedSolomonCodec(int nsym = 10, QObject* parent = nullptr);

    /** @brief 编码 @param data 消息数据 @return 编码后数据(消息+校验) */
    QVector<quint8> encode(const QVector<quint8>& data);

    /** @brief 解码 @param data 接收数据(含错误) @return 纠正后的消息数据 */
    QVector<quint8> decode(const QVector<quint8>& data);

    /** @brief 计算校验符号 @param data 消息 @return 校验符号 */
    QVector<quint8> calculateSyndromes(const QVector<quint8>& data);

    /** @brief 最大可纠正符号数 @return 纠错能力 */
    int errorCorrectionCapacity() const;

    /** @brief 码字总长(n=k+nsym) @param k 消息长度 @return 码字长度 */
    int codewordLength(int k) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    /** @brief 编码完成 @param msgLen 消息长度 @param codeLen 码字长度 */
    void encoded(int msgLen, int codeLen);
    /** @brief 解码完成 @param corrected 纠正符号数 */
    void decoded(int corrected);
    /** @brief 解码失败 */
    void decodeFailed();

private:
    /* GF(2^8)有限域运算 */
    quint8 gfMul(quint8 a, quint8 b) const;
    quint8 gfDiv(quint8 a, quint8 b) const;
    quint8 gfPow(quint8 a, int n) const;
    quint8 gfInverse(quint8 a) const;
    quint8 gfPolyEval(const QVector<quint8>& poly, quint8 x) const;
    QVector<quint8> gfPolyMul(const QVector<quint8>& a,
                               const QVector<quint8>& b) const;
    QVector<quint8> gfPolyAdd(const QVector<quint8>& a,
                               const QVector<quint8>& b) const;
    QVector<quint8> gfPolyScale(const QVector<quint8>& p, quint8 x) const;

    /** @brief 生成生成多项式 @return g(x) */
    QVector<quint8> generatorPoly() const;

    /** @brief Berlekamp-Massey算法求错误定位多项式 @param syndromes 校验子 @return 错误定位多项式 */
    QVector<quint8> berlekampMassey(const QVector<quint8>& syndromes);

    /** @brief Chien搜索求错误位置 @param sigma 错误定位多项式 @param n 数据长度 @return 错误位置列表 */
    QVector<int> chienSearch(const QVector<quint8>& sigma, int n);

    /** @brief Forney算法求错误值 @param sigma 错误定位多项式 @param syndromes 校验子 @param positions 错误位置 @return 错误值列表 */
    QVector<quint8> forneyAlgorithm(const QVector<quint8>& sigma,
                                     const QVector<quint8>& syndromes,
                                     const QVector<int>& positions);

    int m_nsym;                        ///< 校验符号数
    QVector<quint8> m_expTable;        ///< GF指数表(512项，含冗余)
    QVector<quint8> m_logTable;        ///< GF对数表(256项)
    Stats m_stats;
    double m_timeSum;
};

#endif // REEDSOLOMONCODEC_H
