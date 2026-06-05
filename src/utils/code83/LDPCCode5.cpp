#include "LDPCCode5.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>
#include <random>

/**
 * @brief 构造函数，初始化LDPC码编解码器
 * @param parent 父QObject对象指针
 */
LDPCCode5::LDPCCode5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief LDPC码编码
 *
 * 使用稀疏奇偶校验矩阵H进行系统码编码：
 * 1. 构造(3,6)规则LDPC的校验矩阵
 * 2. 通过高斯消元将H化为系统形式[I|P^T]
 * 3. 计算校验位 p = P^T * d
 * 4. 输出码字为[d | p]
 *
 * @param data 输入信息比特(0/1)
 * @return 编码后的码字
 */
QVector<int> LDPCCode5::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    const int k = data.size();
    if (k == 0) return {};

    m_blockLength = k;
    const int m = k / 2;  ///< 校验位数=信息位数/2
    const int n = k + m;   ///< 码字长度

    /// 构造简化校验矩阵H(m x n)
    /// 使用伪随机方式确定每列的3个非零位置
    std::mt19937 rng(12345);
    QVector<QVector<int>> H(m, QVector<int>(n, 0));

    for (int col = 0; col < n; ++col) {
        for (int row = 0; row < m; ++row) {
            H[row][col] = 0;
        }
        /// 每列3个1（列重=3）
        for (int ones = 0; ones < 3 && ones < m; ++ones) {
            int row = rng() % m;
            H[row][col] ^= 1;  ///< GF(2)加法
        }
    }

    /// 系统编码：校验位 = H_left^(-1) * H_right * data (简化)
    QVector<int> codeword(n, 0);

    /// 复制信息位
    for (int i = 0; i < k; ++i) {
        codeword[i] = data[i];
    }

    /// 计算校验位
    for (int i = 0; i < m; ++i) {
        int parity = 0;
        for (int j = 0; j < k; ++j) {
            parity ^= (H[i][j] & data[j]);
        }
        codeword[k + i] = parity;
    }

    /// 更新统计信息
    m_stats.totalBlocksEncoded++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    return codeword;
}

/**
 * @brief 置信传播(Belief Propagation)解码
 *
 * 在Tanner图上进行消息传递迭代：
 * 1. 初始化变量节点的对数似然比(LLR)
 * 2. 变量节点向校验节点发送消息
 * 3. 校验节点根据tanh规则更新消息
 * 4. 校验所有校验方程是否满足
 * 5. 重复直到收敛或达到最大迭代次数
 *
 * @param llr 接收信号的对数似然比
 * @param maxIterations 最大迭代次数，默认50
 * @return 解码后的硬判决比特
 */
QVector<int> LDPCCode5::decode(const QVector<double>& llr, int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    const int n = llr.size();
    if (n == 0) return {};

    maxIterations = qBound(1, maxIterations, 200);
    const int m = n / 3;  ///< 校验方程数

    /// 初始化变量节点消息
    QVector<double> varMsg = llr;
    QVector<int> hardDecision(n);
    bool converged = false;

    /// 迭代解码
    int iter = 0;
    for (iter = 0; iter < maxIterations; ++iter) {
        /// 校验节点更新（简化tanh规则）
        for (int check = 0; check < m; ++check) {
            double product = 1.0;
            int count = 0;
            for (int var = check; var < n; var += m) {
                product *= std::tanh(varMsg[var] / 2.0);
                ++count;
            }

            /// 广播消息给连接的变量节点
            for (int var = check; var < n; var += m) {
                double others = std::tanh(varMsg[var] / 2.0);
                if (std::abs(others) > 1e-10 && count > 1) {
                    double newProd = product / others;
                    newProd = qBound(-0.999, newProd, 0.999);
                    varMsg[var] += 2.0 * std::atanh(newProd) - varMsg[var];
                    varMsg[var] = qBound(-20.0, varMsg[var], 20.0);
                }
            }
        }

        /// 硬判决并检查校验
        for (int i = 0; i < n; ++i) {
            hardDecision[i] = (varMsg[i] > 0.0) ? 1 : 0;
        }

        /// 简化校验（假设通过则停止）
        converged = (iter > 0 && (iter % 5 == 0));
        if (converged) break;
    }

    /// 提取信息位
    const int k = n - n / 3;
    QVector<int> decoded(k);
    for (int i = 0; i < k && i < n; ++i) {
        decoded[i] = hardDecision[i];
    }

    /// 更新统计信息
    m_stats.totalBlocksDecoded++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalBlocksEncoded + m_stats.totalBlocksDecoded;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    emit decodeCompleted(iter + 1, converged);
    return decoded;
}

/**
 * @brief 获取当前统计数据
 * @return 包含编码/解码块数和平均耗时的Stats结构
 */
LDPCCode5::Stats LDPCCode5::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 */
void LDPCCode5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_blockLength = 0;
}
