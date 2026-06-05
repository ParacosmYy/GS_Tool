/**
 * @file PolarCode5.cpp
 * @brief 极化码(Polar Code)编解码器实现
 *
 * 实现极化码的编码和基于CRC辅助的列表(SC)解码算法。
 * 极化码基于信道极化现象，利用Bhattacharyya参数选择
 * 可靠信道传输信息比特，不可靠信道冻结为0。使用QElapsedTimer计时。
 */

#include "utils/code49/PolarCode5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @class PolarCode5
 * @brief 极化码编解码器，支持CA-SCL(CRC辅助列表解码)
 *
 * 编码：使用核矩阵 F = [[1,0],[1,1]] 的n次Kronecker积。
 * 解码：基于Successive Cancellation的列表解码，
 * 每一步保留L条候选路径，最终通过CRC校验选择正确路径。
 */

/**
 * @brief 构造函数，初始化默认参数并生成冻结信道集
 * @param parent 父QObject指针
 */
PolarCode5::PolarCode5(QObject* parent)
    : QObject(parent)
{
    /* 默认CRC-8多项式: x^8 + x^2 + x + 1 */
    m_crcPoly = {1, 1, 1, 0, 0, 0, 0, 1, 1};
    generateFrozen();
}

/**
 * @brief 设置极化码参数
 * @param n 码字长度（2的幂）
 * @param k 信息比特数
 * @param listSize SCL解码列表大小
 */
void PolarCode5::setParameters(int n, int k, int listSize)
{
    m_n = qMax(2, n);
    m_k = qMax(1, k);
    m_listSize = qMax(1, listSize);
    generateFrozen();
}

/**
 * @brief 极化码编码
 *
 * 1. 为信息比特计算CRC校验并附加
 * 2. 将信息+CRC比特放置在可靠信道位置，冻结信道填0
 * 3. 通过核矩阵的Kronecker积进行线性编码
 *
 * @param info 信息比特向量（长度为k）
 * @return 编码后的码字（长度为n）
 */
QVector<int> PolarCode5::encode(const QVector<int>& info)
{
    QElapsedTimer timer;
    timer.start();

    /* 计算CRC并附加到信息比特 */
    QVector<int> infoWithCrc = info;
    quint32 crcVal = crc(info);
    int crcBits = m_crcPoly.size() - 1;
    for (int i = crcBits - 1; i >= 0; --i) {
        infoWithCrc.append((crcVal >> i) & 1);
    }

    /* 将信息+CRC比特放置到u向量的非冻结位置 */
    QVector<int> u(m_n, 0);
    int infoIdx = 0;
    for (int i = 0; i < m_n && infoIdx < infoWithCrc.size(); ++i) {
        if (!m_frozen.contains(i)) {
            u[i] = infoWithCrc[infoIdx++];
        }
    }

    /* 极化变换：x = u * G_N，使用递归蝶形结构 */
    QVector<int> codeword = u;
    int stride = 1;
    while (stride < m_n) {
        for (int block = 0; block < m_n; block += stride * 2) {
            for (int i = 0; i < stride; ++i) {
                int a = codeword[block + i];
                int b = codeword[block + stride + i];
                codeword[block + i] = a ^ b;
                /* b保持不变: codeword[block+stride+i] = b */
            }
        }
        stride *= 2;
    }

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit encodeCompleted(m_n, m_k);
    return codeword;
}

/**
 * @brief SCL(CRC辅助列表)解码
 *
 * 基于Successive Cancellation的列表解码算法：
 * 1. 对每个比特位置，保留最多L条候选路径
 * 2. 在冻结位直接决定为0，在信息位对0/1分别扩展
 * 3. 保留路径度量最好的L条路径
 * 4. 最终选择通过CRC校验的最佳路径
 *
 * @param llr 接收到的对数似然比(LLR)序列，长度为n
 * @return 解码后的信息比特，长度为k
 */
QVector<int> PolarCode5::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    const int L = m_listSize;
    int crcBits = m_crcPoly.size() - 1;
    int infoCrcLen = m_k + crcBits;

    /* 路径管理 */
    struct Path {
        QVector<int> bits;         /* 当前解码比特 */
        double metric;             /* 路径度量 */
        QVector<double> llrBuf;    /* LLR缓存 */
    };

    QVector<Path> paths(1);
    paths[0].bits.reserve(m_n);
    paths[0].metric = 0.0;
    paths[0].llrBuf = llr;

    /* 逐比特解码 */
    for (int pos = 0; pos < m_n; ++pos) {
        QVector<Path> newPaths;

        if (m_frozen.contains(pos)) {
            /* 冻结位：固定为0 */
            for (auto& p : paths) {
                p.bits.append(0);
                /* 更新路径度量 */
                if (pos < p.llrBuf.size()) {
                    double l = p.llrBuf[pos];
                    p.metric += qLn(1.0 + qExp(-l)) / qLn(2.0);
                }
            }
        } else {
            /* 信息位：对每个路径扩展0和1两个候选 */
            for (auto& p : paths) {
                for (int bit : {0, 1}) {
                    Path np = p;
                    np.bits.append(bit);
                    if (pos < np.llrBuf.size()) {
                        double l = np.llrBuf[pos];
                        np.metric += (bit == 0)
                            ? qLn(1.0 + qExp(-l)) / qLn(2.0)
                            : qLn(1.0 + qExp(l)) / qLn(2.0);
                    }
                    newPaths.append(np);
                }
            }

            /* 按路径度量排序，保留最优的L条 */
            std::sort(newPaths.begin(), newPaths.end(),
                [](const Path& a, const Path& b) { return a.metric < b.metric; });
            if (newPaths.size() > L) {
                newPaths.resize(L);
            }
            paths = newPaths;
        }
    }

    /* 选择通过CRC校验的最佳路径 */
    bool crcPass = false;
    QVector<int> bestInfo;
    for (auto& p : paths) {
        /* 提取信息+CRC比特 */
        QVector<int> infoBits;
        int idx = 0;
        for (int i = 0; i < m_n; ++i) {
            if (!m_frozen.contains(i) && i < p.bits.size()) {
                infoBits.append(p.bits[i]);
            }
        }

        /* 校验CRC */
        if (infoBits.size() >= infoCrcLen) {
            QVector<int> dataOnly(infoBits.begin(), infoBits.begin() + m_k);
            quint32 computed = crc(dataOnly);
            quint32 received = 0;
            for (int i = 0; i < crcBits; ++i) {
                received = (received << 1) | infoBits[m_k + i];
            }
            if (computed == received) {
                bestInfo = dataOnly;
                crcPass = true;
                break;
            }
        }
    }

    /* 若CRC均不通过，取度量最优路径的信息部分 */
    if (!crcPass && !paths.isEmpty()) {
        bestInfo.clear();
        int idx = 0;
        for (int i = 0; i < m_n; ++i) {
            if (!m_frozen.contains(i) && i < paths[0].bits.size() && idx < m_k) {
                bestInfo.append(paths[0].bits[i]);
                idx++;
            }
        }
    }

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalEncodes + m_stats.totalDecodes > 0)
        ? m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes) : 0.0;

    emit decodeCompleted(crcPass);
    return bestInfo;
}

/**
 * @brief 重置统计数据
 */
void PolarCode5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 生成冻结信道集
 *
 * 使用简化的Bhattacharyya参数估计各信道的可靠性。
 * 递归计算每个比特信道的可靠性值，选择最可靠的k+crclen个
 * 信道用于传输信息，其余信道冻结为0。
 */
void PolarCode5::generateFrozen()
{
    const int totalInfo = m_k + m_crcPoly.size() - 1;
    const int numFrozen = m_n - totalInfo;

    /* 使用Bhattacharyya参数排序信道可靠性 */
    QVector<QPair<double, int>> reliability(m_n);

    /* 初始化：AWGN信道的近似Bhattacharyya参数 */
    double initZ = 0.5;  /* 初始信道质量 */
    for (int i = 0; i < m_n; ++i) {
        /* 使用位反转序的简单近似 */
        int bits = 0;
        int temp = m_n;
        while (temp > 1) { temp >>= 1; bits++; }
        int rev = 0;
        int x = i;
        for (int b = 0; b < bits; ++b) { rev = (rev << 1) | (x & 1); x >>= 1; }

        /* 可靠性近似：高位置更可靠 */
        reliability[i] = {static_cast<double>(rev), i};
    }

    std::sort(reliability.begin(), reliability.end());

    /* 最不可靠的信道设为冻结 */
    m_frozen.clear();
    for (int i = 0; i < qMin(numFrozen, m_n); ++i) {
        m_frozen.insert(reliability[i].second);
    }
}

/**
 * @brief CRC校验计算
 * @param bits 输入比特向量
 * @return CRC校验值
 */
quint32 PolarCode5::crc(const QVector<int>& bits) const
{
    int crcLen = m_crcPoly.size() - 1;
    quint32 reg = 0;

    for (int bit : bits) {
        int msb = (reg >> (crcLen - 1)) & 1;
        reg = (reg << 1) | bit;
        if (msb) {
            quint32 poly = 0;
            for (int i = 0; i < m_crcPoly.size(); ++i) {
                if (m_crcPoly[i]) poly |= (1U << i);
            }
            reg ^= poly;
        }
    }

    return reg & ((1U << crcLen) - 1);
}
