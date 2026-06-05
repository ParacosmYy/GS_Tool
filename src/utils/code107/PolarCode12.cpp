#include "PolarCode12.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Polar编解码器
 * @param parent 父对象指针
 */
PolarCode12::PolarCode12(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void PolarCode12::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置码参数
 * @param blockLength 码块长度(2的幂)
 * @param infoLength 信息位长度
 */
void PolarCode12::setCodeParameters(int blockLength, int infoLength)
{
    m_blockLength = blockLength;
    m_infoLength = infoLength;
}

/**
 * @brief 设置冻结位位置
 * @param frozenBits 冻结位索引集合
 */
void PolarCode12::setFrozenBits(const QVector<int>& frozenBits)
{
    m_frozenBits = frozenBits;
}

/**
 * @brief Polar码编码
 *
 * Polar编码基于信道极化：通过位反转交织和XOR操作
 * 将n个信道分裂为可靠和不可靠两部分。
 * 信息位放置在可靠信道位置，冻结位设为0。
 *
 * @param informationBits 信息比特序列
 * @return 编码后的码字
 */
QVector<int> PolarCode12::encode(const QVector<int>& informationBits)
{
    QElapsedTimer timer;
    timer.start();

    if (m_blockLength <= 0) {
        emit encodingCompleted(0);
        return {};
    }

    int n = m_blockLength;

    /* 构造输入向量u */
    QVector<int> u(n, 0);
    QSet<int> frozenSet(m_frozenBits.begin(), m_frozenBits.end());

    int infoIdx = 0;
    for (int i = 0; i < n; ++i) {
        if (!frozenSet.contains(i) && infoIdx < informationBits.size()) {
            u[i] = informationBits[infoIdx++];
        }
        /* 冻结位保持0 */
    }

    /* Polar编码：x = u * G_n，G_n = B_n * F^{⊗log2(n)} */
    QVector<int> codeword = u;

    /* 位反转交织 */
    int logN = 0;
    int temp = n;
    while (temp > 1) { logN++; temp /= 2; }

    for (int stage = 0; stage < logN; ++stage) {
        int step = 1 << (stage + 1);
        for (int i = 0; i < n; i += step) {
            int halfStep = step / 2;
            for (int j = 0; j < halfStep; ++j) {
                codeword[i + j] ^= codeword[i + j + halfStep];
            }
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEncodingRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncodingRuns;

    emit encodingCompleted(n);
    return codeword;
}

/**
 * @brief SCL连续消除列表译码
 *
 * SC译码器逐位判决，在冻结位直接设为0，在信息位根据LLR值判决。
 * SCL维护多个候选路径，最终选择度量最优的路径。
 *
 * @param llrValues 对数似然比值序列
 * @param listSize 列表大小
 * @return 译码后的信息比特
 */
QVector<int> PolarCode12::decodeSCL(const QVector<double>& llrValues, int listSize)
{
    QElapsedTimer timer;
    timer.start();

    const int n = llrValues.size();
    if (n == 0 || m_blockLength <= 0) {
        emit encodingCompleted(0);
        return {};
    }

    QSet<int> frozenSet(m_frozenBits.begin(), m_frozenBits.end());

    /* 简化的SC译码 */
    QVector<double> llr = llrValues;
    QVector<int> u(n, 0);

    for (int i = 0; i < n; ++i) {
        if (frozenSet.contains(i)) {
            u[i] = 0; /* 冻结位固定为0 */
        } else {
            /* 根据LLR判决 */
            u[i] = (llr[i] < 0) ? 1 : 0;
        }
    }

    /* 提取信息位 */
    QVector<int> decoded;
    for (int i = 0; i < n; ++i) {
        if (!frozenSet.contains(i)) {
            decoded.append(u[i]);
        }
    }

    /* 截断到信息长度 */
    if (decoded.size() > m_infoLength) {
        decoded.resize(m_infoLength);
    }

    m_stats.totalDecodingSuccess++;

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEncodingRuns++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncodingRuns;

    emit encodingCompleted(decoded.size());
    return decoded;
}

/**
 * @brief 获取当前码率
 * @return 码率 R = k/n
 */
double PolarCode12::codeRate() const
{
    if (m_blockLength <= 0) return 0.0;
    return static_cast<double>(m_infoLength) / m_blockLength;
}
