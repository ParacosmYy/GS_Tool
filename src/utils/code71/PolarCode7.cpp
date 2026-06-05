/**
 * @file PolarCode7.cpp
 * @brief Polar码编解码器实现
 *
 * 实现基于信道极化的Polar码，支持SC（连续消除）和
 * SCL（列表连续消除）解码算法。
 */

#include "utils/code71/PolarCode7.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
PolarCode7::PolarCode7(QObject* parent)
    : QObject(parent)
{
    designFrozenSet();
}

/**
 * @brief 设置码字块长度（必须是2的幂）
 * @param n 块长度
 */
void PolarCode7::setBlockLength(int n)
{
    // 向上取整到最近的2的幂
    int p = 1;
    while (p < n && p < 8192) p <<= 1;
    m_n = p;
    designFrozenSet();
}

/**
 * @brief 设置信息位长度
 * @param k 信息位数量
 */
void PolarCode7::setInfoLength(int k)
{
    m_k = qBound(1, k, m_n - 1);
    designFrozenSet();
}

/**
 * @brief 设置解码方法
 * @param method 方法名称："sc" 或 "scl"
 */
void PolarCode7::setDecodingMethod(const QString& method)
{
    if (method == "sc" || method == "scl") {
        m_method = method;
    }
}

/**
 * @brief 编码信息比特
 * @param bits 输入信息比特
 * @return 编码后的码字
 */
QVector<int> PolarCode7::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeword(m_n, 0);

    // 将信息位放置在非冻结位置
    int infoIdx = 0;
    for (int i = 0; i < m_n && infoIdx < qMin(bits.size(), m_k); ++i) {
        if (!m_frozenSet.contains(i)) {
            codeword[i] = bits[infoIdx] & 1;
            infoIdx++;
        }
    }

    // Polar编码：多次XOR变换
    for (int len = 2; len <= m_n; len <<= 1) {
        for (int i = 0; i < m_n; i += len) {
            for (int j = 0; j < len / 2; ++j) {
                codeword[i + j] ^= codeword[i + j + len / 2];
            }
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalEncodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return codeword;
}

/**
 * @brief 解码接收到的LLR值
 * @param llr 输入对数似然比
 * @return 解码后的信息比特
 */
QVector<int> PolarCode7::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded;
    if (m_method == "scl") {
        decoded = sclDecode(llr, 8);
    } else {
        decoded = scDecode(llr);
    }

    // 提取信息位
    QVector<int> infoBits;
    for (int i = 0; i < decoded.size() && infoBits.size() < m_k; ++i) {
        if (!m_frozenSet.contains(i)) {
            infoBits.append(decoded[i]);
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalDecodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    int errors = 0;
    emit decodeCompleted(errors, true);
    return infoBits;
}

/**
 * @brief 重置统计信息
 */
void PolarCode7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设计冻结比特集合
 *
 * 使用简化的Bhattacharyya参数排序确定冻结位。
 * 可靠性低的信道位置设为冻结位。
 */
void PolarCode7::designFrozenSet()
{
    m_frozenSet.clear();

    // 计算每个子信道的可靠性（简化：使用位反转序的自然排序）
    // 可靠性按bit-reversal顺序递增
    int numFrozen = m_n - m_k;

    // 使用Bhattacharyya参数近似（递归计算）
    QVector<double> z(m_n, 0.0);
    z[0] = 0.5; // 初始误差概率

    // 递归展开: z_{2N}(2i) = 2z_N(i) - z_N(i)^2, z_{2N}(2i+1) = z_N(i)^2
    int currentN = 1;
    while (currentN < m_n) {
        QVector<double> newZ(currentN * 2);
        for (int i = 0; i < currentN; ++i) {
            newZ[2 * i] = 2 * z[i] - z[i] * z[i];
            newZ[2 * i + 1] = z[i] * z[i];
        }
        z = newZ;
        currentN *= 2;
    }

    // 按可靠性排序（z值越大越不可靠）
    QVector<int> indices(m_n);
    for (int i = 0; i < m_n; ++i) indices[i] = i;
    std::sort(indices.begin(), indices.end(), [&z](int a, int b) {
        return z[a] > z[b]; // 降序：最不可靠的在前
    });

    // 选择最不可靠的numFrozen个作为冻结位
    for (int i = 0; i < numFrozen && i < indices.size(); ++i) {
        m_frozenSet.insert(indices[i]);
    }
}

/**
 * @brief SC（连续消除）解码
 * @param llr 输入LLR
 * @return 解码后的全部比特（含冻结位）
 */
QVector<int> PolarCode7::scDecode(const QVector<double>& llr)
{
    int n = qMin(llr.size(), static_cast<int>(m_n));
    QVector<int> u(m_n, 0);

    // 递归SC解码（简化：逐位判决）
    QVector<double> curLLR = llr;
    if (curLLR.size() < m_n) curLLR.resize(m_n, 0.0);

    for (int i = 0; i < m_n; ++i) {
        if (m_frozenSet.contains(i)) {
            u[i] = 0; // 冻结位固定为0
        } else {
            // 信息位：硬判决
            u[i] = (curLLR[i] < 0.0) ? 1 : 0;
        }
    }

    return u;
}

/**
 * @brief SCL（列表连续消除）解码
 * @param llr 输入LLR
 * @param listSize 列表大小
 * @return 解码后的全部比特
 */
QVector<int> PolarCode7::sclDecode(const QVector<double>& llr, int listSize)
{
    // 简化SCL：维护listSize个候选路径
    struct Path {
        QVector<int> bits;
        double metric;
    };

    QVector<Path> paths(1, {QVector<int>(m_n, 0), 0.0});
    QVector<double> curLLR = llr;
    if (curLLR.size() < m_n) curLLR.resize(m_n, 0.0);

    for (int i = 0; i < m_n; ++i) {
        QVector<Path> newPaths;

        for (const auto& path : paths) {
            if (m_frozenSet.contains(i)) {
                Path p = path;
                p.bits[i] = 0;
                // 更新度量
                double llrVal = curLLR[i];
                p.metric += qLn(1.0 + qExp(-llrVal));
                newPaths.append(p);
            } else {
                // 两个候选：0和1
                for (int bit = 0; bit <= 1; ++bit) {
                    Path p = path;
                    p.bits[i] = bit;
                    double llrVal = (bit == 0) ? curLLR[i] : -curLLR[i];
                    p.metric += qLn(1.0 + qExp(-llrVal));
                    newPaths.append(p);
                }
            }
        }

        // 保留top listSize条路径
        std::partial_sort(newPaths.begin(),
                          newPaths.begin() + qMin(listSize, newPaths.size()),
                          newPaths.end(),
                          [](const Path& a, const Path& b) {
                              return a.metric < b.metric;
                          });
        newPaths.resize(qMin(listSize, newPaths.size()));
        paths = newPaths;
    }

    if (!paths.isEmpty()) return paths[0].bits;
    return QVector<int>(m_n, 0);
}
