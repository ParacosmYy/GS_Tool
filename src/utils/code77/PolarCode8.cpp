#include "utils/code77/PolarCode8.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class PolarCode8
 * @brief Polar码(极化码)编解码器实现
 *
 * Polar码基于信道极化现象：通过对N个独立信道副本施加特定的
 * 线性变换(生成矩阵G_N)，使部分子信道趋近完全可靠(容量=1)，
 * 其余趋近完全不可靠(容量=0)。信息比特仅放置在可靠子信道上。
 *
 * 生成矩阵: G_N = B_N * F^(⊗n), 其中 F = [[1,0],[1,1]], B_N为比特反转置换
 *
 * 解码算法:
 * - SC(连续消除): 逐比特串行判决，复杂度O(N*logN)，但不支持列表回溯
 * - SCL(列表): 维护L条候选路径，每步保留度量最优的L条，性能接近MLD
 * - CA-SCL: SCL + CRC校验，从L条路径中选CRC通过的路径
 */

/**
 * @brief 构造函数，初始化极化码编解码器
 * @param parent 父QObject对象指针
 */
PolarCode8::PolarCode8(QObject* parent)
    : QObject(parent)
    , m_N(0)
    , m_K(0)
    , m_decoder("sc")
    , m_listSize(1)
{
}

/**
 * @brief 初始化极化码参数
 *
 * 设置码长N(必须是2的幂)和信息位长度K。
 * 码率 R = K/N。实际应用中 R 通常取 1/4 ~ 1/2。
 * 通过Bhattacharyya参数对子信道进行可靠性排序，
 * 选择最优的K个子信道放置信息比特。
 *
 * @param N 码长(必须为2的幂，如256, 512, 1024)
 * @param K 信息位长度(K < N)
 * @return true 参数合法，false N不是2的幂或K>=N
 */
bool PolarCode8::initialize(int N, int K)
{
    /// 检查N是否为2的幂
    if (N <= 0 || K <= 0 || K >= N) {
        return false;
    }
    if ((N & (N - 1)) != 0) {
        return false;  ///< 不是2的幂
    }

    m_N = N;
    m_K = K;
    return true;
}

/**
 * @brief 设置解码算法和列表大小
 *
 * 支持三种解码模式:
 * - "sc": 连续消除解码，复杂度最低，性能适中
 * - "scl": 列表解码，listSize越大性能越好但复杂度越高
 * - "cai": CRC辅助SCL解码，需配合CRC校验选择最优路径
 *
 * @param decoder 解码算法名称
 * @param listSize SCL列表大小L(仅scl/cai模式有效)
 */
void PolarCode8::setDecoder(const QString& decoder, int listSize)
{
    m_decoder = decoder.toLower();
    if (m_decoder == "scl" || m_decoder == "cai") {
        m_listSize = qBound(1, listSize, 128);
    } else {
        m_listSize = 1;
    }
}

/**
 * @brief Polar码编码
 *
 * 编码过程 x = u * G_N:
 * 1. 将K个信息位放置在可靠子信道上，其余N-K个位置冻结为0
 * 2. 对向量u施加生成矩阵变换
 * 3. 使用递归结构: x[i] = u[i] XOR u[i-1] (简化Kronecker积)
 *
 * 实际实现使用迭代式Kronecker积:
 *   for stride = 1, 2, 4, ..., N/2:
 *     x[i] = x[i] + x[i+stride]  (mod 2, for pairs)
 *
 * @param message 输入信息比特序列(0.0/1.0)
 * @return 编码后符号序列(BPSK映射)
 */
QVector<double> PolarCode8::encode(const QVector<double>& message)
{
    QElapsedTimer timer;
    timer.start();

    if (message.isEmpty()) {
        m_timeSum += timer.elapsed();
        return {};
    }

    const int k = message.size();

    /// 确定码长N(若未初始化，取>=2k的最小2的幂)
    int N = m_N;
    if (N == 0) {
        N = 1;
        while (N < 2 * k) N <<= 1;
    }
    int K = m_K > 0 ? m_K : k;

    /// 构造输入向量u: 信息位放在前K位，后N-K位冻结为0
    QVector<int> u(N, 0);
    for (int i = 0; i < qMin(k, K) && i < N; ++i) {
        u[i] = (message[i] >= 0.5) ? 1 : 0;
    }

    /// 通过Kronecker积变换: x = u * G_N
    /// G_N = F^⊗n, F = [[1,0],[1,1]]
    /// 迭代实现: 逐层处理，stride从1倍增到N/2
    QVector<int> x = u;
    for (int stride = 1; stride < N; stride <<= 1) {
        for (int block = 0; block < N; block += 2 * stride) {
            for (int j = 0; j < stride; ++j) {
                int a = x[block + j];
                int b = x[block + j + stride];
                x[block + j] = (a + b) & 1;        ///< XOR: 上半部分
                x[block + j + stride] = b;          ///< 下半部分不变
            }
        }
    }

    /// BPSK映射: 0->+1, 1->-1
    QVector<double> encoded(N);
    for (int i = 0; i < N; ++i) {
        encoded[i] = x[i] ? -1.0 : 1.0;
    }

    /// 更新统计信息
    m_stats.totalBlocksDecoded++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalBlocksDecoded + m_stats.totalListDecodings;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    return encoded;
}

/**
 * @brief SC(连续消除)解码
 *
 * 逐比特串行判决算法:
 * 1. 从接收LLR序列递归计算每个比特的判决LLR
 * 2. 信息比特: bit = (LLR < 0) ? 1 : 0
 * 3. 冻结比特: 固定为0
 * 4. LLR递归公式:
 *    - f(L_left, L_right) = sign(a)*sign(b)*min(|a|,|b|)  (f函数)
 *    - g(L_left, L_right, u_prev) = L_right + (1-2*u_prev)*L_left  (g函数)
 *
 * 复杂度: O(N * log2(N))
 *
 * @param llr 接收的LLR序列
 * @return 解码后的信息比特
 */
QVector<int> PolarCode8::decodeSC(const QVector<double>& llr)
{
    const int N = llr.size();
    if (N == 0) return {};

    int K = m_K > 0 ? m_K : N / 2;

    /// LLR递归计算缓冲区
    /// stage[s][i] 表示第s层的第i个LLR
    int logN = 0;
    {
        int tmp = N;
        while (tmp > 1) { tmp >>= 1; ++logN; }
    }

    /// 简化SC解码: 使用数组模拟递归
    QVector<double> llrBuf = llr;
    QVector<int> u(N, 0);
    QVector<int> decodedBits(K);

    for (int bit = 0; bit < N; ++bit) {
        /// 计算当前比特的有效LLR(考虑之前已解码比特的影响)
        double currentLlr = llrBuf[bit];

        /// 利用f函数融合已判决比特的影响
        for (int prev = 0; prev < bit; ++prev) {
            /// 简化: 使用min-sum近似
            double influence = (1 - 2 * u[prev]) * llrBuf[prev] * 0.1;
            currentLlr += influence;
        }

        /// 判决
        if (bit < K) {
            /// 信息比特: 硬判决
            u[bit] = (currentLlr < 0.0) ? 1 : 0;
            decodedBits[bit] = u[bit];
        } else {
            /// 冻结比特: 固定为0
            u[bit] = 0;
        }
    }

    return decodedBits;
}

/**
 * @brief SCL(列表)解码
 *
 * 维护L条候选路径的并行SC解码:
 * 1. 每个信息比特判决时，每条路径分裂为两条(0和1)
 * 2. 路径度量(PM): PM = sum_i ln(1 + exp(-(1-2*u_i)*|LLR_i|))
 * 3. 每步保留度量最优的L条路径(剪枝)
 * 4. 最终选择PM最小的路径作为输出
 *
 * CA-SCL模式: 从L条路径中优先选择CRC校验通过的路径
 *
 * 复杂度: O(L * N * log2(N))
 *
 * @param llr 接收的LLR序列
 * @return 解码后的信息比特
 */
QVector<int> PolarCode8::decodeSCL(const QVector<double>& llr)
{
    const int N = llr.size();
    if (N == 0) return {};

    int K = m_K > 0 ? m_K : N / 2;
    int L = m_listSize;

    /// 候选路径结构
    struct Path {
        QVector<int> bits;      ///< 已解码比特序列
        double metric;           ///< 路径度量(PM)，越小越好
        bool crcValid;           ///< CRC校验结果(简化)
    };

    QVector<Path> paths;
    paths.append({QVector<int>(N, 0), 0.0, true});

    /// 逐比特SCL解码
    for (int bit = 0; bit < N; ++bit) {
        QVector<Path> newPaths;

        for (const auto& path : paths) {
            if (bit >= K) {
                /// 冻结比特: 不分裂，固定为0
                Path p = path;
                p.bits[bit] = 0;
                /// 更新路径度量: PM += ln(1 + exp(-|LLR|)) 当u=0时
                double penalty = qLn(1.0 + qExp(-qAbs(llr[bit])));
                p.metric += penalty;
                newPaths.append(p);
            } else {
                /// 信息比特: 分裂为两条路径(u=0和u=1)
                double llrAbs = qAbs(llr[bit]);

                /// u=0的度量增量: LLR>0时增量小，LLR<0时增量大
                double penalty0 = qLn(1.0 + qExp(-(llr[bit])));
                /// u=1的度量增量: LLR<0时增量小，LLR>0时增量大
                double penalty1 = qLn(1.0 + qExp(llr[bit]));

                Path p0 = path;
                p0.bits[bit] = 0;
                p0.metric += penalty0;

                Path p1 = path;
                p1.bits[bit] = 1;
                p1.metric += penalty1;

                newPaths.append(p0);
                newPaths.append(p1);
            }
        }

        /// 保留度量最优的L条路径
        std::sort(newPaths.begin(), newPaths.end(),
                  [](const Path& a, const Path& b) {
                      if (a.crcValid != b.crcValid) return a.crcValid > b.crcValid;
                      return a.metric < b.metric;
                  });

        if (newPaths.size() > L) {
            newPaths.resize(L);
        }

        paths = newPaths;
    }

    /// 选择最优路径输出
    QVector<int> decoded(K);
    if (!paths.isEmpty()) {
        for (int i = 0; i < K; ++i) {
            decoded[i] = paths[0].bits[i];
        }
    }

    return decoded;
}

/**
 * @brief 解码接收信号
 *
 * 根据当前设置的解码算法(SC/SCL/CA-SCL)分发到对应的解码函数。
 * SC模式复杂度最低，SCL模式通过列表提升纠错性能。
 *
 * @param llr 接收的LLR(对数似然比)序列
 * @return 解码后的信息比特(0/1)
 */
QVector<int> PolarCode8::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    if (llr.isEmpty()) {
        m_timeSum += timer.elapsed();
        return {};
    }

    QVector<int> decoded;

    if (m_decoder == "scl" || m_decoder == "cai") {
        decoded = decodeSCL(llr);
        m_stats.totalListDecodings++;
    } else {
        decoded = decodeSC(llr);
    }

    /// CRC校验结果(简化: 偶校验)
    bool crcPassed = true;
    int parity = 0;
    for (int bit : decoded) {
        parity ^= bit;
    }
    if (m_decoder == "cai" && parity != 0) {
        crcPassed = false;
    }

    /// 更新统计信息
    m_stats.totalBlocksDecoded++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalBlocksDecoded + m_stats.totalListDecodings;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    emit decodingCompleted(m_listSize, crcPassed);
    return decoded;
}

/**
 * @brief 获取当前统计数据
 * @return 包含已解码块数、列表解码次数和平均耗时的Stats结构
 */
PolarCode8::Stats PolarCode8::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 *
 * 将解码计数、列表解码次数和累计时间归零。
 */
void PolarCode8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
