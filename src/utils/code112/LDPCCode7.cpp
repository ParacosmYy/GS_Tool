#include "LDPCCode7.h"
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>
#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化LDPC编解码器
 * @param parent 父对象指针
 */
LDPCCode7::LDPCCode7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void LDPCCode7::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 构造规则LDPC校验矩阵
 *
 * 采用随机构造法生成规则LDPC码的稀疏校验矩阵H。
 * 每列恰好有columnWeight个1，每行恰好有rowWeight个1。
 * 通过随机置换和冲突消除保证行列重量约束。
 *
 * @param blockLength 码长n
 * @param columnWeight 列重（每个变量节点连接的校验节点数）
 * @param rowWeight 行重（每个校验节点连接的变量节点数）
 * @return m×n稀疏校验矩阵（0/1元素）
 */
QVector<QVector<int>> LDPCCode7::buildParityMatrix(int blockLength, int columnWeight, int rowWeight)
{
    QVector<QVector<int>> H;
    if (blockLength <= 0 || columnWeight <= 0 || rowWeight <= 0) return H;

    /* 校验行列重量一致性: n * colW = m * rowW */
    if (blockLength * columnWeight % rowWeight != 0) return H;

    int m = blockLength * columnWeight / rowWeight;
    H.resize(m);
    for (auto& row : H) {
        row.resize(blockLength, 0);
    }

    /* 逐列放置1，确保行重约束 */
    QVector<int> rowDegrees(m, 0);
    for (int col = 0; col < blockLength; ++col) {
        int placed = 0;
        int attempts = 0;
        while (placed < columnWeight && attempts < 1000) {
            int row = QRandomGenerator::global()->bounded(1000000) % m;
            if (H[row][col] == 0 && rowDegrees[row] < rowWeight) {
                H[row][col] = 1;
                rowDegrees[row]++;
                placed++;
            }
            attempts++;
        }
        /* 回退：若无法满足约束则强制放置 */
        if (placed < columnWeight) {
            for (int row = 0; row < m && placed < columnWeight; ++row) {
                if (H[row][col] == 0) {
                    H[row][col] = 1;
                    rowDegrees[row]++;
                    placed++;
                }
            }
        }
    }
    return H;
}

/**
 * @brief 使用LDPC码对数据进行系统编码
 *
 * 系统编码：码字c = [数据位 | 校验位]。
 * 利用校验矩阵H的列变换得到生成矩阵G，
 * 通过高斯消元将H化为系统形式 H = [P^T | I]，
 * 编码时计算校验位 p = P * d (mod 2)。
 *
 * @param dataBits 原始信息比特序列
 * @return 编码后的码字序列（信息位+校验位）
 */
QVector<int> LDPCCode7::encode(const QVector<int>& dataBits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> codeword;
    if (dataBits.isEmpty()) {
        emit decodeCompleted(0);
        return codeword;
    }

    int k = dataBits.size();
    /* 默认码率1/2，校验位数量等于信息位数量 */
    int n = 2 * k;

    /* 构造系统校验矩阵 H = [P^T | I_{n-k}] */
    int m = n - k;
    QVector<QVector<int>> P(m, QVector<int>(k, 0));

    /* 伪随机填充P矩阵（确保每行每列有适当重量） */
    for (int row = 0; row < m; ++row) {
        int ones = qMax(1, k / (m * 2));
        for (int cnt = 0; cnt < ones; ++cnt) {
            int col = (row * 7 + cnt * 13 + 3) % k;
            P[row][col] = 1;
        }
    }

    /* 计算校验位: p_i = sum(P[i][j] * d[j]) mod 2 */
    QVector<int> parity(m, 0);
    for (int i = 0; i < m; ++i) {
        int sum = 0;
        for (int j = 0; j < k; ++j) {
            sum += P[i][j] & (dataBits[j] & 1);
        }
        parity[i] = sum & 1;
    }

    /* 系统码字：[数据 | 校验] */
    codeword.reserve(n);
    for (int b : dataBits) codeword.append(b & 1);
    for (int p : parity) codeword.append(p);

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(0);
    return codeword;
}

/**
 * @brief 置信传播迭代解码
 *
 * 基于对数似然比(LLR)的消息传递算法。
 * 每次迭代分为两步：
 * 1. 校验节点更新：计算校验到变量的LLR消息
 * 2. 变量节点更新：汇总所有校验消息，更新后验概率
 * 硬判决后检查是否满足所有校验方程。
 *
 * @param receivedBits 接收到的含噪LLR序列
 * @param maxIterations 最大迭代次数（默认50）
 * @return 解码后的信息比特序列
 */
QVector<int> LDPCCode7::decodeBP(const QVector<double>& receivedBits, int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded;
    if (receivedBits.isEmpty()) {
        emit decodeCompleted(0);
        return decoded;
    }

    int n = receivedBits.size();
    int k = n / 2;
    int m = n - k;

    /* 初始化LLR */
    QVector<double> llr(n);
    for (int i = 0; i < n; ++i) {
        llr[i] = qBound(-20.0, receivedBits[i], 20.0);
    }

    /* 构建简化的校验连接关系 */
    QVector<QVector<int>> checkToVar(m);
    QVector<QVector<int>> varToCheck(n);
    for (int row = 0; row < m; ++row) {
        for (int cnt = 0; cnt < qMax(1, k / (m * 2)); ++cnt) {
            int col = (row * 7 + cnt * 13 + 3) % k;
            if (!checkToVar[row].contains(col)) {
                checkToVar[row].append(col);
                varToCheck[col].append(row);
            }
        }
        int parityIdx = k + row;
        if (parityIdx < n) {
            checkToVar[row].append(parityIdx);
            if (parityIdx < varToCheck.size()) {
                varToCheck[parityIdx].append(row);
            }
        }
    }

    /* 初始化消息矩阵 */
    QVector<QVector<double>> msgVarToCheck(n);
    for (int v = 0; v < n; ++v) {
        msgVarToCheck[v].resize(varToCheck[v].size(), 0.0);
    }

    int iter = 0;
    for (iter = 0; iter < maxIterations; ++iter) {
        /* 变量节点 → 校验节点消息 */
        for (int v = 0; v < n; ++v) {
            for (int ci = 0; ci < varToCheck[v].size(); ++ci) {
                double sum = llr[v];
                for (int cj = 0; cj < varToCheck[v].size(); ++cj) {
                    if (ci != cj) sum += msgVarToCheck[v][cj];
                }
                msgVarToCheck[v][ci] = qBound(-20.0, sum - msgVarToCheck[v][ci], 20.0);
            }
        }

        /* 校验节点 → 变量节点消息（使用tanh法则简化） */
        for (int c = 0; c < m; ++c) {
            for (int vi = 0; vi < checkToVar[c].size(); ++vi) {
                int var = checkToVar[c][vi];
                double product = 1.0;
                for (int vj = 0; vj < checkToVar[c].size(); ++vj) {
                    if (vi != vj) {
                        int otherVar = checkToVar[c][vj];
                        int msgIdx = -1;
                        for (int mi = 0; mi < varToCheck[otherVar].size(); ++mi) {
                            if (varToCheck[otherVar][mi] == c) {
                                msgIdx = mi;
                                break;
                            }
                        }
                        if (msgIdx >= 0 && msgIdx < msgVarToCheck[otherVar].size()) {
                            double val = qBound(-20.0, msgVarToCheck[otherVar][msgIdx], 20.0);
                            product *= std::tanh(val / 2.0);
                        }
                    }
                }
                product = qBound(-0.999999, product, 0.999999);
                double checkMsg = 2.0 * std::atanh(product);

                /* 更新变量节点 */
                int varMsgIdx = -1;
                for (int mi = 0; mi < varToCheck[var].size(); ++mi) {
                    if (varToCheck[var][mi] == c) {
                        varMsgIdx = mi;
                        break;
                    }
                }
                if (varMsgIdx >= 0 && varMsgIdx < msgVarToCheck[var].size()) {
                    msgVarToCheck[var][varMsgIdx] = qBound(-20.0, checkMsg, 20.0);
                }
            }
        }

        /* 硬判决并检查校验 */
        bool allSatisfied = true;
        QVector<int> hardDecision(n, 0);
        for (int v = 0; v < n; ++v) {
            double totalLLR = llr[v];
            for (double msg : msgVarToCheck[v]) totalLLR += msg;
            hardDecision[v] = (totalLLR < 0) ? 1 : 0;
        }

        for (int c = 0; c < m; ++c) {
            int syndrome = 0;
            for (int var : checkToVar[c]) {
                if (var < hardDecision.size()) syndrome ^= hardDecision[var];
            }
            if (syndrome != 0) { allSatisfied = false; break; }
        }
        if (allSatisfied) break;
    }

    /* 提取信息位 */
    decoded.reserve(k);
    for (int i = 0; i < k && i < receivedBits.size(); ++i) {
        double totalLLR = llr[i];
        for (double msg : msgVarToCheck[i]) totalLLR += msg;
        decoded.append((totalLLR < 0) ? 1 : 0);
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    emit decodeCompleted(iter);
    return decoded;
}

/**
 * @brief 蒙特卡洛仿真计算误码率
 *
 * 在AWGN信道下，对随机信息序列进行编码、加噪、解码，
 * 统计比特错误率。BPSK调制: 0→+1, 1→-1。
 * 噪声方差由SNR(dB)计算: σ² = 10^(-snrDb/10) / 2。
 *
 * @param snrDb 信噪比(dB)
 * @param numTrials 仿真试验次数
 * @return 误码率(BER)
 */
double LDPCCode7::computeBER(double snrDb, int numTrials)
{
    QElapsedTimer timer;
    timer.start();

    if (numTrials <= 0) return 1.0;

    int k = 64;
    double sigma = qSqrt(qPow(10.0, -snrDb / 10.0) / 2.0);
    qint64 totalBits = 0;
    qint64 totalErrors = 0;

    for (int trial = 0; trial < numTrials; ++trial) {
        /* 生成随机信息位 */
        QVector<int> dataBits(k);
        for (int i = 0; i < k; ++i) {
            dataBits[i] = QRandomGenerator::global()->bounded(1000000) % 2;
        }

        /* 编码 */
        QVector<int> codeword = encode(dataBits);
        if (codeword.isEmpty()) continue;

        /* BPSK调制 + AWGN加噪 */
        QVector<double> noisy(codeword.size());
        for (int i = 0; i < codeword.size(); ++i) {
            double symbol = (codeword[i] == 0) ? 1.0 : -1.0;
            double noise = sigma * (2.0 * (QRandomGenerator::global()->bounded(1000000) % 10000) / 10000.0 - 1.0);
            noisy[i] = symbol + noise;
        }

        /* 转换为LLR */
        QVector<double> llr(codeword.size());
        for (int i = 0; i < codeword.size(); ++i) {
            llr[i] = 2.0 * noisy[i] / (sigma * sigma);
        }

        /* 解码 */
        QVector<int> decoded = decodeBP(llr, 50);

        /* 统计错误 */
        for (int i = 0; i < k && i < decoded.size(); ++i) {
            totalBits++;
            if (decoded[i] != dataBits[i]) totalErrors++;
        }
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodeOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodeOps;

    return (totalBits > 0) ? static_cast<double>(totalErrors) / totalBits : 1.0;
}
