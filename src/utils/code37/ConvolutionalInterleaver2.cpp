/**
 * @file ConvolutionalInterleaver2.cpp
 * @brief 卷积交织器实现 — 支持可配置分支数与延迟深度
 *
 * 实现卷积交织/解交织功能，广泛用于通信系统中的突发错误保护。
 * 核心原理: 数据按行写入、按列读出(交织)，通过FIFO移位寄存器
 * 实现不同分支的延迟差异，打乱原始数据顺序以分散突发错误。
 *
 * 特性:
 * - 可配置分支数(B)和延迟单元(D)
 * - 支持交织与解交织操作(对称结构)
 * - FIFO移位寄存器自动管理
 * - 统计交织次数、处理符号数、平均耗时
 */

#include "utils/code37/ConvolutionalInterleaver2.h"

#include <QElapsedTimer>

/* ===== 公有方法实现 ===== */

/**
 * @brief 构造函数 — 初始化卷积交织器
 * @param parent QObject父对象
 */
ConvolutionalInterleaver2::ConvolutionalInterleaver2(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/**
 * @brief 配置交织器参数 — 设置分支数和延迟深度
 *
 * 交织器结构: B个分支(分支0~B-1)
 * 分支i的FIFO深度为 i*D (分支0无延迟，直接通过)
 * 总延迟 = B * (B-1) * D / 2 个符号周期
 *
 * @param branches 分支数B (>= 1)
 * @param delay 每级延迟深度D (>= 1)
 */
void ConvolutionalInterleaver2::configure(int branches, int delay)
{
    m_branches = qMax(1, branches);
    m_delay = qMax(1, delay);

    /* 初始化FIFO移位寄存器 */
    /* 分支i有 i*D 个延迟单元 */
    m_fifo.resize(m_branches);
    for (int i = 0; i < m_branches; ++i) {
        int fifoDepth = i * m_delay;
        m_fifo[i].resize(fifoDepth);
        m_fifo[i].fill(0);
    }
}

/**
 * @brief 交织操作 — 将输入数据按卷积交织方式打乱顺序
 *
 * 处理流程:
 * 1. 逐符号输入，按分支号循环分配
 * 2. 分支0: 直接通过(无延迟)
 * 3. 分支i(i>0): 数据压入FIFO，从FIFO尾部弹出旧数据
 * 4. 输出为经过不同延迟后的符号序列
 *
 * @param data 输入字节流
 * @return 交织后的字节流
 */
QByteArray ConvolutionalInterleaver2::interleave(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (m_branches <= 0 || data.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalInterleaves++;
        m_stats.avgProcessingTimeMs =
            (m_stats.totalInterleaves > 0) ? m_timeSum / m_stats.totalInterleaves : 0.0;
        return QByteArray();
    }

    QByteArray output;
    output.resize(data.size());

    for (int i = 0; i < data.size(); ++i) {
        int branch = i % m_branches;
        quint8 symbol = static_cast<quint8>(data[i]);

        if (branch == 0) {
            /* 分支0: 无延迟，直接输出 */
            output[i] = static_cast<char>(symbol);
        } else {
            /* 分支i: FIFO移位 — 新数据入头，旧数据从尾出 */
            int fifoDepth = branch * m_delay;
            quint8 oldSymbol = m_fifo[branch][fifoDepth - 1];

            /* 移位: 从尾向前移动 */
            for (int j = fifoDepth - 1; j > 0; --j) {
                m_fifo[branch][j] = m_fifo[branch][j - 1];
            }
            m_fifo[branch][0] = symbol;

            output[i] = static_cast<char>(oldSymbol);
        }
    }

    /* 更新统计信息 */
    m_stats.totalInterleaves++;
    m_stats.totalSymbolsProcessed += data.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalInterleaves > 0) ? m_timeSum / m_stats.totalInterleaves : 0.0;

    emit interleaved(data.size());
    return output;
}

/**
 * @brief 解交织操作 — 将交织后的数据恢复为原始顺序
 *
 * 解交织器结构与交织器对称:
 * 分支i的FIFO深度为 (B-1-i)*D
 * 分支0延迟最大，分支B-1无延迟
 * 总延迟与交织器相同，保证端到端延迟一致
 *
 * @param data 交织后的字节流
 * @return 解交织后的原始字节流
 */
QByteArray ConvolutionalInterleaver2::deinterleave(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (m_branches <= 0 || data.isEmpty()) {
        m_timeSum += timer.elapsed();
        m_stats.totalInterleaves++;
        m_stats.avgProcessingTimeMs =
            (m_stats.totalInterleaves > 0) ? m_timeSum / m_stats.totalInterleaves : 0.0;
        return QByteArray();
    }

    /* 初始化解交织FIFO — 与交织器对称 */
    QVector<QVector<quint8>> deintFifo(m_branches);
    for (int i = 0; i < m_branches; ++i) {
        /* 解交织分支i的FIFO深度 = (B-1-i)*D */
        int fifoDepth = (m_branches - 1 - i) * m_delay;
        deintFifo[i].resize(fifoDepth);
        deintFifo[i].fill(0);
    }

    QByteArray output;
    output.resize(data.size());

    for (int i = 0; i < data.size(); ++i) {
        int branch = i % m_branches;
        quint8 symbol = static_cast<quint8>(data[i]);

        int fifoDepth = (m_branches - 1 - branch) * m_delay;

        if (fifoDepth == 0) {
            /* 无延迟分支: 直接输出 */
            output[i] = static_cast<char>(symbol);
        } else {
            /* FIFO移位 — 新数据入头，旧数据从尾出 */
            quint8 oldSymbol = deintFifo[branch][fifoDepth - 1];

            for (int j = fifoDepth - 1; j > 0; --j) {
                deintFifo[branch][j] = deintFifo[branch][j - 1];
            }
            deintFifo[branch][0] = symbol;

            output[i] = static_cast<char>(oldSymbol);
        }
    }

    /* 更新统计信息 */
    m_stats.totalInterleaves++;
    m_stats.totalSymbolsProcessed += data.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        (m_stats.totalInterleaves > 0) ? m_timeSum / m_stats.totalInterleaves : 0.0;

    emit interleaved(data.size());
    return output;
}

/**
 * @brief 重置交织器 — 清空所有FIFO移位寄存器
 */
void ConvolutionalInterleaver2::reset()
{
    for (int i = 0; i < m_fifo.size(); ++i) {
        m_fifo[i].fill(0);
    }
}

/**
 * @brief 重置统计信息
 */
void ConvolutionalInterleaver2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
