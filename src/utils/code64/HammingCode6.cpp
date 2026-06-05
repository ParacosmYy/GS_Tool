/**
 * @file HammingCode6.cpp
 * @brief 汉明码编解码器实现（第6版）
 *
 * 实现系统汉明码的编码和解码。支持可配置冗余位数，
 * 自动构建校验矩阵H和生成矩阵G。解码时可检测并纠正
 * 单比特错误，检测双比特错误。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/code64/HammingCode6.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化汉明码编解码器
 * @param parent 父QObject对象指针
 */
HammingCode6::HammingCode6(QObject* parent)
    : QObject(parent)
{
    buildParityCheckMatrix();
    buildGeneratorMatrix();
}

/**
 * @brief 设置冗余位数，重建矩阵
 * @param r 校验位数（码长n=2^r-1，数据位k=n-r）
 */
void HammingCode6::setRedundancy(int r)
{
    m_r = qMax(2, r);
    m_n = (1 << m_r) - 1;
    m_k = m_n - m_r;
    buildParityCheckMatrix();
    buildGeneratorMatrix();
}

/**
 * @brief 构建校验矩阵H
 *
 * H矩阵大小为r×n，每列为1到n的二进制表示。
 * 第i列的内容为i的二进制表示（低位在上）。
 */
void HammingCode6::buildParityCheckMatrix()
{
    m_H.resize(m_r);
    for (int i = 0; i < m_r; ++i) {
        m_H[i].resize(m_n, 0);
    }

    for (int col = 0; col < m_n; ++col) {
        int val = col + 1;  /* 列号为1~n */
        for (int row = 0; row < m_r; ++row) {
            m_H[row][col] = (val >> row) & 1;
        }
    }
}

/**
 * @brief 构建生成矩阵G
 *
 * G矩阵大小为k×n，系统形式G = [I_k | P^T]。
 * 校验位放在2的幂次位置（1,2,4,8,...），数据位放在其余位置。
 */
void HammingCode6::buildGeneratorMatrix()
{
    m_G.resize(m_k);
    for (int i = 0; i < m_k; ++i) {
        m_G[i].resize(m_n, 0);
    }

    /* 确定数据位和校验位的位置 */
    QVector<int> dataPositions;
    QVector<int> parityPositions;
    for (int i = 1; i <= m_n; ++i) {
        if ((i & (i - 1)) == 0) {
            /* 2的幂次位置为校验位 */
            parityPositions.append(i - 1); /* 转为0索引 */
        } else {
            dataPositions.append(i - 1);
        }
    }

    /* 构建系统形式生成矩阵 */
    for (int i = 0; i < m_k; ++i) {
        /* 单位矩阵部分 */
        m_G[i][dataPositions[i]] = 1;

        /* 校验部分：数据位i参与哪些校验位的计算 */
        int col1based = dataPositions[i] + 1;
        for (int p = 0; p < m_r; ++p) {
            if (col1based & (1 << p)) {
                m_G[i][parityPositions[p]] = 1;
            }
        }
    }
}

/**
 * @brief 编码数据比特
 *
 * 使用生成矩阵G对k位数据进行编码，产生n位码字。
 * c = d * G（模2运算）
 *
 * @param data 输入的k位数据
 * @return 编码后的n位码字
 */
QVector<int> HammingCode6::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> code(m_n, 0);

    if (data.size() != m_k) {
        /* 数据长度不匹配，尝试截断或填充 */
        QVector<int> padded(m_k, 0);
        for (int i = 0; i < qMin(data.size(), m_k); ++i) {
            padded[i] = data[i] & 1;
        }
        for (int i = 0; i < m_k; ++i) {
            for (int j = 0; j < m_n; ++j) {
                code[j] ^= (padded[i] & m_G[i][j]);
            }
        }
    } else {
        for (int i = 0; i < m_k; ++i) {
            for (int j = 0; j < m_n; ++j) {
                code[j] ^= ((data[i] & 1) & m_G[i][j]);
            }
        }
    }

    /* 更新统计 */
    m_stats.totalEncodes++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncodes + m_stats.totalDecodes);

    return code;
}

/**
 * @brief 解码接收到的码字
 *
 * 计算伴随式s = r * H^T。若s=0则无错误；
 * 否则s的非零值指示错误位置（二进制编码的列号）。
 * 可纠正单比特错误。
 *
 * @param received 接收到的n位码字
 * @return 解码后的k位数据
 */
QVector<int> HammingCode6::decode(const QVector<int>& received)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> data(m_k, 0);

    if (received.size() != m_n) {
        return data;
    }

    /* 计算伴随式 */
    int syndrome = 0;
    for (int i = 0; i < m_r; ++i) {
        int bit = 0;
        for (int j = 0; j < m_n; ++j) {
            bit ^= (received[j] & m_H[i][j]);
        }
        syndrome |= (bit << i);
    }

    /* 纠错 */
    bool corrected = false;
    QVector<int> correctedWord = received;
    if (syndrome != 0 && syndrome <= m_n) {
        /* 单比特错误，翻转对应位 */
        correctedWord[syndrome - 1] ^= 1;
        corrected = true;
    }

    /* 提取数据位 */
    int dataIdx = 0;
    for (int i = 1; i <= m_n; ++i) {
        /* 非校验位位置的数据 */
        if ((i & (i - 1)) != 0 && dataIdx < m_k) {
            data[dataIdx++] = correctedWord[i - 1];
        }
    }

    /* 更新统计 */
    m_stats.totalDecodes++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(syndrome, corrected);
    return data;
}

/**
 * @brief 获取当前统计信息
 * @return 编解码统计结构
 */
HammingCode6::Stats HammingCode6::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void HammingCode6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
