/**
 * @file ReedSolomon2.h
 * @brief Reed-Solomon纠错码 — GF(2^8)实现
 *
 * 功能: 在GF(2^8)上实现Reed-Solomon编码与解码，
 *       支持可配置校验符号数，统计编码/解码次数/耗时。
 */
#pragma once

#include <QObject>
#include <QByteArray>

class ReedSolomon2 : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalEncoded = 0;       ///< 总编码次数
        quint64 totalDecoded = 0;       ///< 总解码次数
        double  avgProcessingTimeMs = 0.0; ///< 平均处理时间(ms)
    };

    explicit ReedSolomon2(QObject* parent = nullptr);

    /**
     * @brief 设置纠错参数
     * @param nsym 校验符号数量
     */
    void setParameters(int nsym);

    /**
     * @brief 编码数据
     * @param data 原始数据
     * @return 编码后数据(原始+校验)
     */
    QByteArray encode(const QByteArray& data);

    /**
     * @brief 解码数据(纠正错误)
     * @param received 接收到的编码数据
     * @return 解码后的原始数据(去除校验)
     */
    QByteArray decode(const QByteArray& received);

    /** @brief 获取统计信息 */
    const Stats& stats() const { return m_stats; }

    /** @brief 重置统计 */
    void resetStatistics();

signals:
    /** @brief 解码完成信号 @param success 是否成功 @param corrected 纠正的符号数 */
    void decodingCompleted(bool success, int corrected);

private:
    /** @brief GF(2^8)乘法(本原多项式0x11D) */
    quint8 gfMul(quint8 a, quint8 b) const;

    /** @brief GF(2^8)除法 */
    quint8 gfDiv(quint8 a, quint8 b) const;

    /** @brief GF(2^8)求逆 */
    quint8 gfInv(quint8 a) const;

    /** @brief GF(2^8)多项式求值 */
    quint8 gfPolyEval(const QVector<quint8>& poly, quint8 x) const;

    /** @brief GF(2^8)多项式乘法 */
    QVector<quint8> gfPolyMul(const QVector<quint8>& a,
                               const QVector<quint8>& b) const;

    /** @brief 计算伴随式 */
    QVector<quint8> calcSyndromes(const QByteArray& data) const;

    /** @brief Berlekamp-Massey算法求错误定位多项式 */
    QVector<quint8> berlekampMassey(const QVector<quint8>& synd) const;

    /** @brief Chien搜索求错误位置 */
    QVector<int> chienSearch(const QVector<quint8>& errLoc, int dataLen) const;

    /** @brief Forney算法求错误值 */
    QVector<quint8> forneyAlgorithm(const QVector<quint8>& synd,
                                     const QVector<quint8>& errLoc,
                                     const QVector<int>& errPos) const;

    /** @brief GF(2^8)幂运算 */
    quint8 gfPow(quint8 base, int exp) const;

    /** @brief 初始化GF对数/反对数表 */
    void initTables();

    Stats  m_stats;
    double m_timeSum;
    int    m_nsym;              ///< 校验符号数
    bool   m_tablesInit;       ///< 查找表是否已初始化
    quint8 m_gfExp[512];       ///< GF指数表
    quint8 m_gfLog[256];       ///< GF对数表
};
