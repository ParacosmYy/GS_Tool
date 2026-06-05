/**
 * @file ReedSolomon2.h
 * @brief Reed-Solomon编解码增强 — BM/Forney/擦除/Chien搜索
 *
 * 基于有限域GF(2^m)的Reed-Solomon编解码器，支持:
 *   - Berlekamp-Massey算法求解错误位置多项式
 *   - Forney公式计算错误值
 *   - 擦除解码(已知错误位置)
 *   - Chien搜索定位错误
 * 统计编码/解码操作次数和纠错成功率。
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QElapsedTimer>

/**
 * @brief Reed-Solomon编解码增强
 */
class ReedSolomon2 : public QObject {
    Q_OBJECT
public:
    /** 统计信息 */
    struct Stats {
        quint64 totalEncodes = 0;          ///< 总编码次数
        quint64 totalDecodes = 0;          ///< 总解码次数
        quint64 totalCorrectedErrors = 0;  ///< 总纠正错误数
        quint64 totalUncorrectable = 0;    ///< 总不可纠正次数
        double avgProcessingTimeMs = 0.0;  ///< 平均处理耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param symbolSize 符号位数m(通常8, 即GF(256))
     * @param nsym 校验符号数(纠错能力 = nsym/2)
     * @param primitive 本原多项式(默认0x11D用于GF(256))
     * @param parent 父对象
     */
    explicit ReedSolomon2(int symbolSize = 8, int nsym = 10,
                          int primitive = 0x11D, QObject* parent = nullptr);

    /**
     * @brief 编码 — 生成校验符号
     * @param data 输入数据
     * @return 编码后的完整码字(数据+校验)
     */
    QVector<int> encode(const QVector<int>& data);

    /**
     * @brief 解码 — 纠错并恢复原始数据
     * @param received 接收到的码字(可能含错)
     * @param erasePos 已知擦除位置(可选)
     * @return 纠错后的数据部分，失败返回空
     */
    QVector<int> decode(const QVector<int>& received, const QVector<int>& erasePos = {});

    /** @brief 获取统计 */
    const Stats& stats() const { return m_stats; }
    /** @brief 重置统计 */
    void resetStatistics();

    /** @brief 最大可纠正错误数 */
    int maxCorrectableErrors() const { return m_nsym / 2; }
    /** @brief 校验符号数 */
    int parityCount() const { return m_nsym; }

signals:
    /** 编码完成 */
    void encoded(int dataLen, int parityLen);
    /** 解码完成 */
    void decoded(int correctedErrors, bool success);

private:
    /** 初始化GF指数/对数表 */
    void initGaloisField();
    /** GF乘法 */
    int gfMul(int a, int b) const;
    /** GF除法 */
    int gfDiv(int a, int b) const;
    /** GF幂运算 */
    int gfPow(int a, int n) const;
    /** GF逆元 */
    int gfInv(int a) const;
    /** 计算Syndrome */
    QVector<int> calcSyndromes(const QVector<int>& data) const;
    /** Berlekamp-Massey算法 */
    QVector<int> berlekampMassey(const QVector<int>& synd, int nsym) const;
    /** Chien搜索 */
    QVector<int> chienSearch(const QVector<int>& errLoc, int n) const;
    /** Forney公式求错误值 */
    QVector<int> forneyAlgorithm(const QVector<int>& synd,
                                 const QVector<int>& errLoc,
                                 const QVector<int>& errPos) const;

    int m_symbolSize;
    int m_nsym;
    int m_primitive;
    int m_fieldSize;
    QVector<int> m_gfExp;
    QVector<int> m_gfLog;
    Stats m_stats;
    double m_timeSum = 0.0;
    QElapsedTimer m_timing;
};
