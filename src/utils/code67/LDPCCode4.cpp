/**
 * @file LDPCCode4.cpp
 * @brief 低密度奇偶校验码(LDPC)编解码器实现
 *
 * 实现基于稀疏校验矩阵的LDPC编解码，支持MinSum和SumProduct两种
 * 解码算法。适用于通信系统中的前向纠错编码。
 */

#include "utils/code67/LDPCCode4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>

/**
 * @brief 构造函数，初始化默认参数并加载校验矩阵
 * @param parent 父对象指针
 */
LDPCCode4::LDPCCode4(QObject* parent)
    : QObject(parent)
{
    loadParityCheckMatrix();
}

/**
 * @brief 设置码字块长度
 * @param n 块长度（码字长度）
 */
void LDPCCode4::setBlockLength(int n)
{
    m_n = qMax(16, n);
    loadParityCheckMatrix();
}

/**
 * @brief 设置码率
 * @param rate 码率，范围(0, 1)
 */
void LDPCCode4::setCodeRate(double rate)
{
    m_rate = qBound(0.1, rate, 0.95);
    loadParityCheckMatrix();
}

/**
 * @brief 设置解码算法
 * @param algo 算法名称："minsum" 或 "sumproduct"
 */
void LDPCCode4::setDecodingAlgorithm(const QString& algo)
{
    if (algo == "minsum" || algo == "sumproduct") {
        m_algo = algo;
    }
}

/**
 * @brief 编码信息比特
 * @param bits 输入信息比特向量
 * @return 编码后的码字
 */
QVector<int> LDPCCode4::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    int k = qRound(m_n * m_rate);
    QVector<int> codeword(m_n, 0);

    // 系统码：前k位为信息位
    for (int i = 0; i < qMin(bits.size(), k); ++i) {
        codeword[i] = bits[i] & 1;
    }

    // 使用校验矩阵的行计算校验位
    int m = m_n - k;
    for (int i = 0; i < m && i < m_H.size(); ++i) {
        int parity = 0;
        for (int col : m_H[i]) {
            if (col < codeword.size()) {
                parity ^= codeword[col];
            }
        }
        codeword[k + i] = parity;
    }

    m_stats.totalEncodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return codeword;
}

/**
 * @brief 解码接收到的LLR值
 * @param llr 输入的对数似然比
 * @return 解码后的硬判决比特
 */
QVector<int> LDPCCode4::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    if (m_algo == "minsum") {
        result = minSumDecode(llr);
    } else {
        result = sumProductDecode(llr);
    }

    // 硬判决
    QVector<int> decoded(result.size());
    for (int i = 0; i < result.size(); ++i) {
        decoded[i] = (result[i] < 0.0) ? 1 : 0;
    }

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    // 检查是否收敛（所有校验方程是否满足）
    bool converged = true;
    int m = m_H.size();
    for (int i = 0; i < m && converged; ++i) {
        int check = 0;
        for (int col : m_H[i]) {
            if (col < decoded.size()) check ^= decoded[col];
        }
        if (check != 0) converged = false;
    }

    emit decodeCompleted(50, converged);
    return decoded;
}

/**
 * @brief 重置统计信息
 */
void LDPCCode4::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 加载/生成稀疏校验矩阵
 *
 * 使用规则准循环构造法生成校验矩阵，每行有固定数量的非零元素。
 */
void LDPCCode4::loadParityCheckMatrix()
{
    int k = qRound(m_n * m_rate);
    int m = m_n - k;
    int rowWeight = 6; // 每行非零元素数
    int colWeight = qRound(static_cast<double>(m * rowWeight) / m_n);

    m_H.clear();
    m_Hcol.clear();
    m_Hcol.resize(m_n);

    std::mt19937 rng(12345);
    int blockRows = m / 3;
    int blockCols = m_n / 3;

    for (int i = 0; i < m; ++i) {
        QVector<int> row;
        // 均匀分布在块内
        int baseCol = (i % 3) * blockCols;
        for (int j = 0; j < rowWeight; ++j) {
            int col = (baseCol + j * blockCols / rowWeight + rng() % qMax(1, blockCols / rowWeight / 2)) % m_n;
            if (!row.contains(col)) {
                row.append(col);
            }
        }
        // 确保足够的非零元素
        while (row.size() < qMin(rowWeight, m_n)) {
            int col = rng() % m_n;
            if (!row.contains(col)) row.append(col);
        }
        std::sort(row.begin(), row.end());
        m_H.append(row);

        // 记录列索引
        for (int col : row) {
            if (col < m_Hcol.size() && !m_Hcol[col].contains(i)) {
                m_Hcol[col].append(i);
            }
        }
    }
}

/**
 * @brief MinSum解码算法
 * @param llr 输入对数似然比
 * @return 解码后的软判决LLR
 *
 * 使用最小和近似代替SumProduct中的双曲正切运算，
 * 计算复杂度更低，性能略有损失。
 */
QVector<double> LDPCCode4::minSumDecode(const QVector<double>& llr)
{
    const int maxIter = 50;
    const double scaleFactor = 0.75; // 归一化因子

    int n = qMin(llr.size(), m_n);
    int m = m_H.size();

    // 初始化变量节点到校验节点的消息
    QVector<QVector<double>> varToCheck(m);
    QVector<QVector<double>> checkToVar(m);
    for (int i = 0; i < m; ++i) {
        int deg = m_H[i].size();
        varToCheck[i].resize(deg, 0.0);
        checkToVar[i].resize(deg, 0.0);
        for (int j = 0; j < deg; ++j) {
            int col = m_H[i][j];
            varToCheck[i][j] = (col < n) ? llr[col] : 0.0;
        }
    }

    QVector<double> appLlr(n);
    for (int iter = 0; iter < maxIter; ++iter) {
        // 校验节点更新（MinSum规则）
        for (int i = 0; i < m; ++i) {
            int deg = m_H[i].size();
            for (int j = 0; j < deg; ++j) {
                double minVal = 1e18;
                double signProd = 1.0;
                for (int jj = 0; jj < deg; ++jj) {
                    if (jj == j) continue;
                    double val = varToCheck[i][jj];
                    signProd *= (val >= 0.0) ? 1.0 : -1.0;
                    minVal = qMin(minVal, qAbs(val));
                }
                checkToVar[i][j] = signProd * minVal * scaleFactor;
            }
        }

        // 变量节点更新
        for (int col = 0; col < n && col < m_Hcol.size(); ++col) {
            double total = (col < llr.size()) ? llr[col] : 0.0;
            for (int rowIdx : m_Hcol[col]) {
                int j = m_H[rowIdx].indexOf(col);
                if (j >= 0) total += checkToVar[rowIdx][j];
            }
            appLlr[col] = total;

            // 更新该变量节点到所有相关校验节点的消息
            for (int rowIdx : m_Hcol[col]) {
                int j = m_H[rowIdx].indexOf(col);
                if (j >= 0) {
                    varToCheck[rowIdx][j] = total - checkToVar[rowIdx][j];
                }
            }
        }

        // 硬判决检查校验方程
        bool allSatisfied = true;
        for (int i = 0; i < m && allSatisfied; ++i) {
            int parity = 0;
            for (int col : m_H[i]) {
                if (col < n && appLlr[col] < 0.0) parity ^= 1;
            }
            if (parity != 0) allSatisfied = false;
        }
        if (allSatisfied) break;
    }

    return appLlr;
}

/**
 * @brief SumProduct解码算法
 * @param llr 输入对数似然比
 * @return 解码后的软判决LLR
 *
 * 标准置信传播算法，使用tanh运算进行校验节点更新。
 */
QVector<double> LDPCCode4::sumProductDecode(const QVector<double>& llr)
{
    const int maxIter = 50;
    int n = qMin(llr.size(), m_n);
    int m = m_H.size();

    QVector<QVector<double>> varToCheck(m);
    QVector<QVector<double>> checkToVar(m);
    for (int i = 0; i < m; ++i) {
        int deg = m_H[i].size();
        varToCheck[i].resize(deg, 0.0);
        checkToVar[i].resize(deg, 0.0);
        for (int j = 0; j < deg; ++j) {
            int col = m_H[i][j];
            varToCheck[i][j] = (col < n) ? llr[col] : 0.0;
        }
    }

    QVector<double> appLlr(n);
    for (int iter = 0; iter < maxIter; ++iter) {
        // 校验节点更新（tanh规则）
        for (int i = 0; i < m; ++i) {
            int deg = m_H[i].size();
            for (int j = 0; j < deg; ++j) {
                double prodTan = 1.0;
                for (int jj = 0; jj < deg; ++jj) {
                    if (jj == j) continue;
                    double val = varToCheck[i][jj];
                    prodTan *= qTanh(qBound(-30.0, val * 0.5, 30.0));
                }
                prodTan = qBound(-1.0 + 1e-10, prodTan, 1.0 - 1e-10);
                checkToVar[i][j] = 2.0 * qAtanh(prodTan);
            }
        }

        // 变量节点更新
        for (int col = 0; col < n && col < m_Hcol.size(); ++col) {
            double total = (col < llr.size()) ? llr[col] : 0.0;
            for (int rowIdx : m_Hcol[col]) {
                int j = m_H[rowIdx].indexOf(col);
                if (j >= 0) total += checkToVar[rowIdx][j];
            }
            appLlr[col] = total;
            for (int rowIdx : m_Hcol[col]) {
                int j = m_H[rowIdx].indexOf(col);
                if (j >= 0) varToCheck[rowIdx][j] = total - checkToVar[rowIdx][j];
            }
        }

        bool allSatisfied = true;
        for (int i = 0; i < m && allSatisfied; ++i) {
            int parity = 0;
            for (int col : m_H[i]) {
                if (col < n && appLlr[col] < 0.0) parity ^= 1;
            }
            if (parity != 0) allSatisfied = false;
        }
        if (allSatisfied) break;
    }

    return appLlr;
}
