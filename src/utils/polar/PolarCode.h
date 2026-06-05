/**
 * @file PolarCode.h
 * @brief Polar码编解码器 — 冻结位/SC解码/信道极化
 *
 * 提供Polar码(极化码)的编码器和SC(连续消除)解码器实现,
 * 支持自动冻结位选择(基于巴塔恰里亚参数)、CRC辅助解码。
 * 适用于嵌入式调试中的通信协议编码和信道编码仿真。
 */
#ifndef POLAR_CODE_H
#define POLAR_CODE_H

#include <QObject>
#include <QVector>
#include <QByteArray>

/**
 * @class PolarCode
 * @brief Polar码编解码器
 *
 * Polar码是唯一可证明达到信道容量的线性分组码。
 * 码长 N=2^n, 信息位 K, 冻结位 N-K。
 */
class PolarCode : public QObject {
    Q_OBJECT

public:
    /** @brief 操作统计结构 */
    struct Stats {
        quint64 totalEncodes = 0;       ///< 编码操作总次数
        quint64 totalDecodes = 0;       ///< 解码操作总次数
        quint64 totalBits = 0;          ///< 处理的比特总数
        double  avgDecodeTimeMs = 0.0;  ///< 平均解码耗时(ms)
    };

    /**
     * @brief 构造函数
     * @param n 码长指数(N=2^n), 默认8(N=256)
     * @param k 信息位数, 默认128
     * @param parent 父对象
     */
    explicit PolarCode(int n = 8, int k = 128, QObject* parent = nullptr);

    /** @brief 析构函数 */
    ~PolarCode() override;

    // ── 编码 ──

    /**
     * @brief Polar码编码
     * @param infoBits 信息比特(长度=K)
     * @return 编码后比特(长度=N)
     */
    QVector<quint8> encode(const QVector<quint8>& infoBits);

    // ── 解码 ──

    /**
     * @brief SC(连续消除)解码
     * @param receivedBits 接收比特(长度=N, 软判决: 0/1)
     * @return 解码后的信息比特
     */
    QVector<quint8> decodeSC(const QVector<quint8>& receivedBits);

    /**
     * @brief SC解码(软判决LLR输入)
     * @param llr 对数似然比(长度=N)
     * @return 解码后的信息比特
     */
    QVector<quint8> decodeSCLlr(const QVector<double>& llr);

    // ── 配置 ──

    /** @brief 获取码长 N=2^n */
    int blockLength() const;

    /** @brief 获取信息位数 K */
    int infoLength() const;

    /** @brief 获取码率 R=K/N */
    double codeRate() const;

    /** @brief 获取冻结位位置列表 */
    QVector<int> frozenPositions() const;

    // ── 统计 ──

    /** @brief 获取当前统计数据快照 */
    Stats stats() const;

    /** @brief 重置统计计数器归零 */
    void resetStatistics();

signals:
    /** @brief 编码完成信号 @param n 码长 */
    void encoded(int n);
    /** @brief 解码完成信号 @param k 信息位数 */
    void decoded(int k);
    /** @brief 错误信号 @param msg 错误描述 */
    void error(const QString& msg);

private:
    /** @brief 计算巴塔恰里亚参数(确定冻结位) */
    void computeReliability();

    /** @brief 选择冻结位(可靠性最低的N-K位) */
    void selectFrozenBits();

    /** @brief Polar变换(递归) */
    void polarTransform(QVector<quint8>& bits) const;

    /** @brief SC解码递归 */
    void scDecodeRecursive(const QVector<double>& llr,
                           QVector<quint8>& decoded,
                           int offset, int length);

    /** @brief 计算LLR (左子节点) */
    double llrLeft(double a, double b) const;

    /** @brief 计算LLR (右子节点) */
    double llrRight(double a, double b, quint8 u) const;

    int m_n;                            ///< 码长指数
    int m_N;                            ///< 码长 N=2^n
    int m_K;                            ///< 信息位数
    QVector<double> m_reliability;      ///< 信道可靠性序列
    QVector<int> m_frozenPositions;     ///< 冻结位位置
    QSet<int> m_frozenSet;              ///< 冻结位集合(快速查询)

    mutable Stats m_stats;              ///< 操作统计
};

#endif // POLAR_CODE_H
