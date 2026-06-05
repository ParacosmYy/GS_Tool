/**
 * @file PolarCode3.cpp
 * @brief Polar码增强实现 — CA-SCL解码/编码/CRC校验/冻结集管理
 */

#include "utils/code31/PolarCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/** @brief 构造函数 @param parent 父对象 */
PolarCode3::PolarCode3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 配置极化码参数
 * @param n 码长(必须是2的幂)
 * @param k 信息位数
 * @param listSize SCL列表大小
 */
void PolarCode3::configure(int n, int k, int listSize)
{
    m_n = qMax(2, n);
    m_k = qMax(1, k);
    m_listSize = qMax(1, listSize);

    /* 确保n是2的幂 */
    int p = 1;
    while (p < m_n) p <<= 1;
    m_n = p;

    m_k = qMin(m_k, m_n - 1);

    /* 默认冻结集: 低可靠度信道 */
    generateFrozenSet();
}

/** @brief 设置CRC多项式位数 @param crcBits CRC位数(8/16/24) */
void PolarCode3::setCRC(int crcBits)
{
    m_crcBits = qBound(8, crcBits, 32);
}

/**
 * @brief 极化码编码
 * @param message 信息位(k位)
 * @return 编码后码字(n位)
 */
QVector<int> PolarCode3::encode(const QVector<int>& message) const
{
    if (message.size() != m_k || m_n == 0) return {};

    /* 构造u向量: 冻结位填0，信息位填消息 */
    QVector<int> u(m_n, 0);
    int infoIdx = 0;
    for (int i = 0; i < m_n && infoIdx < m_k; ++i) {
        if (!m_frozen.contains(i)) {
            u[i] = message[infoIdx++];
        }
    }

    /* Arikan递归编码: x = G_N * u, G_N = B_N * F^{⊗n} */
    QVector<int> codeword = u;
    for (int stride = 2; stride <= m_n; stride <<= 1) {
        QVector<int> temp = codeword;
        for (int i = 0; i < m_n; i += stride) {
            for (int j = 0; j < stride / 2; ++j) {
                codeword[i + j] = (temp[i + j] + temp[i + j + stride / 2]) % 2;
                codeword[i + j + stride / 2] = temp[i + j + stride / 2];
            }
        }
    }

    return codeword;
}

/**
 * @brief CA-SCL解码
 * @param llr 信道LLR值(n个)
 * @return 解码后的信息位(k位)
 */
QVector<int> PolarCode3::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    if (llr.size() != m_n || m_n == 0) return {};

    /* SC解码为基础 */
    QVector<double> beliefs = llr;
    QVector<int> uHat(m_n, 0);

    /* 逐位SC解码 */
    scDecodeRecursive(beliefs, uHat, 0, m_n);

    /* SCL: 维护多条候选路径 */
    struct Path {
        QVector<int> bits;
        QVector<double> lrs;
        double metric;
        bool active;
    };

    int L = m_listSize;
    QVector<Path> paths(L);
    for (int l = 0; l < L; ++l) {
        paths[l].bits.resize(m_n, 0);
        paths[l].lrs = beliefs;
        paths[l].metric = 0.0;
        paths[l].active = (l == 0);
    }

    int activeCount = 1;

    for (int i = 0; i < m_n; ++i) {
        if (m_frozen.contains(i)) {
            /* 冻结位: 所有活跃路径设为0 */
            for (int l = 0; l < L; ++l) {
                if (paths[l].active) {
                    paths[l].bits[i] = 0;
                }
            }
        } else {
            /* 信息位: 分裂路径 */
            QVector<QPair<double, int>> candidates;
            for (int l = 0; l < L; ++l) {
                if (!paths[l].active) continue;

                double llr_i = paths[l].lrs[i];
                double metric0 = paths[l].metric
                    + qLn(1.0 + qExp(-llr_i));
                double metric1 = paths[l].metric
                    + qLn(1.0 + qExp(llr_i));

                candidates.append(qMakePair(metric0, l * 2));
                candidates.append(qMakePair(metric1, l * 2 + 1));
            }

            /* 选择最优的activeCount条路径 */
            std::sort(candidates.begin(), candidates.end());
            int keep = qMin(2 * activeCount, L);

            QVector<bool> usedPath(L, false);
            QVector<Path> newPaths = paths;

            for (int r = 0; r < keep; ++r) {
                int origL = candidates[r].second / 2;
                int bitVal = candidates[r].second % 2;

                newPaths[r] = paths[origL];
                newPaths[r].bits[i] = bitVal;
                newPaths[r].metric = candidates[r].first;
                newPaths[r].active = true;
            }

            for (int r = keep; r < L; ++r) {
                newPaths[r].active = false;
            }

            paths = newPaths;
            activeCount = keep;
        }

        /* 更新LLR传播 */
        for (int l = 0; l < L; ++l) {
            if (!paths[l].active) continue;
            updateLLR(paths[l].lrs, paths[l].bits, i);
        }
    }

    /* CRC校验选择最优路径 */
    int bestPath = 0;
    double bestMetric = std::numeric_limits<double>::max();
    bool crcOk = false;

    for (int l = 0; l < L; ++l) {
        if (!paths[l].active) continue;
        if (checkCRC(paths[l].bits)) {
            if (paths[l].metric < bestMetric) {
                bestMetric = paths[l].metric;
                bestPath = l;
                crcOk = true;
            }
        }
    }

    /* 若无CRC通过则选度量最优 */
    if (!crcOk) {
        for (int l = 0; l < L; ++l) {
            if (paths[l].active && paths[l].metric < bestMetric) {
                bestMetric = paths[l].metric;
                bestPath = l;
            }
        }
    }

    /* 提取信息位 */
    QVector<int> result;
    result.reserve(m_k);
    for (int i = 0; i < m_n; ++i) {
        if (!m_frozen.contains(i)) {
            result.append(paths[bestPath].bits[i]);
        }
    }

    m_stats.totalDecodes++;
    m_stats.totalBitsProcessed += m_n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalEncodes + m_stats.totalDecodes));

    emit decodeComplete(crcOk);
    return result;
}

/** @brief 设置冻结集 @param frozen 冻结位索引 */
void PolarCode3::setFrozenSet(const QVector<int>& frozen)
{
    m_frozen = frozen;
}

/** @brief 获取码长 @return n */
int PolarCode3::n() const { return m_n; }

/** @brief 获取信息位数 @return k */
int PolarCode3::k() const { return m_k; }

/** @brief 重置统计 */
void PolarCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief SC递归解码
 * @param llr LLR数组
 * @param uHat 解码结果
 * @param start 起始索引
 * @param len 长度
 */
void PolarCode3::scDecodeRecursive(QVector<double>& llr, QVector<int>& uHat,
    int start, int len) const
{
    if (len == 1) {
        /* 叶节点: 硬判决 */
        if (m_frozen.contains(start)) {
            uHat[start] = 0;
        } else {
            uHat[start] = (llr[start] < 0) ? 1 : 0;
        }
        return;
    }

    int half = len / 2;

    /* f运算: 上半部分 */
    QVector<double> llrLeft(half);
    for (int i = 0; i < half; ++i) {
        double a = llr[start + i];
        double b = llr[start + i + half];
        llrLeft[i] = signProd(a, b);
    }

    QVector<double> savedLlr = llr;
    for (int i = 0; i < half; ++i) {
        llr[start + i] = llrLeft[i];
    }

    scDecodeRecursive(llr, uHat, start, half);

    /* g运算: 下半部分 */
    for (int i = 0; i < half; ++i) {
        double a = savedLlr[start + i];
        double b = savedLlr[start + i + half];
        int u = uHat[start + i];
        llr[start + i] = (u == 0) ? b : -b;
    }

    scDecodeRecursive(llr, uHat, start + half, half);
}

/**
 * @brief 近似sign-prod运算
 * @param a LLR值a
 * @param b LLR值b
 * @return min(|a|,|b|) * sign(a) * sign(b)
 */
double PolarCode3::signProd(double a, double b) const
{
    double sign = (a >= 0 ? 1.0 : -1.0) * (b >= 0 ? 1.0 : -1.0);
    return sign * qMin(qFabs(a), qFabs(b));
}

/**
 * @brief CRC校验
 * @param bits 待校验比特序列
 * @return 是否通过
 */
bool PolarCode3::checkCRC(const QVector<int>& bits) const
{
    /* 简化CRC校验: 使用CRC-16-CCITT */
    int dataLen = bits.size() - m_crcBits;
    if (dataLen <= 0) return true;

    quint16 crc = 0xFFFF;
    quint16 poly = 0x1021;

    for (int i = 0; i < dataLen; ++i) {
        quint16 bit = (bits[i] & 1);
        quint16 msb = (crc >> 15) & 1;
        crc <<= 1;
        crc ^= (bit ? 1 : 0);
        if (msb) crc ^= poly;
        crc &= 0xFFFF;
    }

    /* 比较校验位 */
    for (int i = 0; i < m_crcBits && dataLen + i < bits.size(); ++i) {
        int expected = (crc >> (m_crcBits - 1 - i)) & 1;
        if (bits[dataLen + i] != expected) return false;
    }
    return true;
}

/**
 * @brief 更新LLR传播
 * @param llr LLR数组
 * @param bits 当前比特
 * @param idx 当前索引
 */
void PolarCode3::updateLLR(QVector<double>& llr, const QVector<int>& bits, int idx) const
{
    /* 简化LLR更新: 基于已解码比特的先验信息 */
    for (int i = idx + 1; i < m_n; ++i) {
        if (i % 2 == 0 && i + 1 < m_n) {
            /* 偶数位置: f运算更新 */
            double a = llr[i];
            double b = llr[i + 1];
            llr[i] = signProd(a, b);
        }
    }
}

/** @brief 自动生成冻结集(基于Bhattacharyya参数近似) */
void PolarCode3::generateFrozenSet()
{
    if (m_n == 0) return;

    /* 计算每个信道的可靠度(Welch-Bhattacharyya近似) */
    QVector<QPair<double, int>> reliability(m_n);
    for (int i = 0; i < m_n; ++i) {
        /* 使用位反转序的自然排序近似 */
        double bw = bhattacharyya(i);
        reliability[i] = qMakePair(bw, i);
    }

    /* 按可靠度排序: 最差的冻结 */
    std::sort(reliability.begin(), reliability.end());

    m_frozen.clear();
    int frozenCount = m_n - m_k;
    for (int i = 0; i < frozenCount; ++i) {
        m_frozen.append(reliability[i].second);
    }
}

/**
 * @brief 近似Bhattacharyya参数
 * @param index 信道索引
 * @return Bhattacharyya参数值
 */
double PolarCode3::bhattacharyya(int index) const
{
    /* 递推近似: Z(W^-) = 2Z - Z^2, Z(W^+) = Z^2 */
    double z = 0.5; /* 初始信道参数 */
    int n = m_n;

    for (int bit = 0; (1 << bit) <= n; ++bit) {
        if (index & (1 << bit)) {
            z = 2.0 * z - z * z;
        } else {
            z = z * z;
        }
        z = qBound(0.0, z, 1.0);
    }
    return z;
}
