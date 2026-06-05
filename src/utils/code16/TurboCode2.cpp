/**
 * @file TurboCode2.cpp
 * @brief Turbo码编解码器实现 — 并行级联RSC + MAP/Log-MAP/SOVA译码
#include "utils/code16/TurboCode2.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <numeric>
#include <random>
/** @brief 构造函数 @param parent 父对象 */
TurboCode2::TurboCode2(QObject* parent)
    : QObject(parent)
    , m_numStates(0)
    , m_mask(0)
{
    Config defaultCfg;
    defaultCfg.generators = {7, 5}; /* 八进制 7=111b, 5=101b */
    configure(defaultCfg);
}

/** @brief 析构函数 */
TurboCode2::~TurboCode2() = default;
/** @brief 配置编解码参数 @param config 配置 */
void TurboCode2::configure(const Config& config)
{
    m_config = config;
    /* 确保有默认生成多项式 */
    if (m_config.generators.size() < 2) {
        m_config.generators = {7, 5};
    }
    m_numStates = 1 << (m_config.constraintLength - 1);
    m_mask = m_numStates - 1;
    /* 生成交织/解交织表 */
    m_interleaver = generateInterleaver(m_config.interleaverSize);
    m_deinterleaver.resize(m_interleaver.size());
    for (int i = 0; i < m_interleaver.size(); ++i) {
        m_deinterleaver[m_interleaver[i]] = i;
    }
}

/** @brief 获取当前配置 @return 配置 */
TurboCode2::Config TurboCode2::configuration() const
{
    return m_config;
}

/** @brief Turbo编码(比特流) @param bits 输入比特(0/1) @return 编码后比特 */
QVector<int> TurboCode2::encodeBits(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();
    int n = bits.size();
    if (n == 0) return {};
    int numRscOut = m_config.generators.size() - 1; /* 系统位 + (numRscOut)校验 */
    /* RSC编码器1: 系统位 + 校验1 */
    int state1 = 0;
    QVector<int> parity1(n);
    QVector<int> terminatedBits;
    for (int i = 0; i < n; ++i) {
        auto [nextState, parity] = rscEncode(state1, bits[i]);
        parity1[i] = parity;
        state1 = nextState;
    }
    /* 终止RSC1 */
    if (m_config.enableTermination) {
        for (int t = 0; t < m_config.constraintLength - 1; ++t) {
            /* 反馈使状态归零 */
            int feedbackBit = (state1 & 1) ^ bits.size() > 0 ? 0 : 0;
            /* 用反馈比特作为输入强制状态转移 */
            int fb = __builtin_parity(state1 & m_config.generators[0])
                   ^ ((state1 >> (m_config.constraintLength - 2)) & 1);
            auto [ns, p] = rscEncode(state1, fb);
            terminatedBits.append(fb);
            state1 = ns;
        }
    }
    /* 交织输入 */
    QVector<int> interleaved(n);
    for (int i = 0; i < n; ++i) {
        int idx = m_interleaver[i % m_interleaver.size()];
        interleaved[i] = bits[idx % n];
    }
    /* RSC编码器2: 交织后的系统位(不传输) + 校验2 */
    int state2 = 0;
    QVector<int> parity2(n);
    for (int i = 0; i < n; ++i) {
        auto [nextState, parity] = rscEncode(state2, interleaved[i]);
        parity2[i] = parity;
        state2 = nextState;
    }
    /* 复用: sys[i], p1[i], p2[i] */
    QVector<int> encoded;
    encoded.reserve(n * 3 + terminatedBits.size() * 3);
    for (int i = 0; i < n; ++i) {
        encoded.append(bits[i]);       /* 系统位 */
        encoded.append(parity1[i]);    /* 校验1 */
        encoded.append(parity2[i]);    /* 校验2 */
    }
    /* 尾比特 */
    for (int t = 0; t < terminatedBits.size(); ++t) {
        encoded.append(terminatedBits[t]);
        encoded.append(parity1[n + t]); /* 尾部校验 */
        encoded.append(0);               /* RSC2尾校验占位 */
    }
    /* 统计 */
    m_stats.totalEncodes++;
    m_stats.totalBitsEncoded += static_cast<quint64>(n);
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(m_stats.totalEncodes, 1ULL));
    emit encodeCompleted(n, encoded.size());
    return encoded;
}

/** @brief Turbo编码(字节数据) @param data 输入字节数据 @return 编码后比特 */
QVector<int> TurboCode2::encodeBytes(const QByteArray& data)
{
    QVector<int> bits;
    bits.reserve(data.size() * 8);
    for (quint8 byte : data) {
        for (int b = 7; b >= 0; --b) {
            bits.append((byte >> b) & 1);
        }
    }
    return encodeBits(bits);
}

/** @brief 迭代译码(软输入) @param systematic 系统位软值 @param parity1 校验1软值 @param parity2 校验2软值 @return 译码结果 */
TurboCode2::DecodeResult TurboCode2::decode(
    const QVector<double>& systematic,
    const QVector<double>& parity1,
    const QVector<double>& parity2)
{
    QElapsedTimer timer;
    timer.start();
    DecodeResult result;
    int n = systematic.size();
    if (n == 0) return result;
    /* 初始化外信息 */
    QVector<double> extrinsic(n, 0.0);
    QVector<double> extrinsicIntlv(n, 0.0);
    double prevMetric = 1e30;
    int numStates = m_numStates;
    for (int iter = 0; iter < m_config.maxIterations; ++iter) {
        QVector<double> input1(n);
        for (int i = 0; i < n; ++i) {
            input1[i] = systematic[i] + extrinsic[i];
        }
        QVector<double> app1, ext1;
        switch (m_config.algorithm) {
        case MAP:
            std::tie(app1, ext1) = decodeMAP(input1, parity1, extrinsic);
            break;
        case SOVA:
            std::tie(app1, ext1) = decodeSOVA(input1, parity1, extrinsic);
            break;
        case LogMAP:
        default:
            std::tie(app1, ext1) = decodeLogMAP(input1, parity1, extrinsic);
            break;
        }
        /* 交织外信息 */
        QVector<double> extIntlv(n);
        for (int i = 0; i < n; ++i) {
            int idx = m_interleaver[i % m_interleaver.size()] % n;
            extIntlv[idx] = ext1[i];
        }
        QVector<double> input2(n);
        for (int i = 0; i < n; ++i) {
            /* 交织后的系统位 */
            int srcIdx = m_interleaver[i % m_interleaver.size()] % n;
            input2[i] = systematic[srcIdx] + extIntlv[i];
        }
        QVector<double> app2, ext2;
        switch (m_config.algorithm) {
        case MAP:
            std::tie(app2, ext2) = decodeMAP(input2, parity2, extIntlv);
            break;
        case SOVA:
            std::tie(app2, ext2) = decodeSOVA(input2, parity2, extIntlv);
            break;
        case LogMAP:
        default:
            std::tie(app2, ext2) = decodeLogMAP(input2, parity2, extIntlv);
            break;
        }
        /* 解交织外信息 */
        for (int i = 0; i < n; ++i) {
            int idx = m_deinterleaver[i % m_deinterleaver.size()] % n;
            extrinsic[idx] = ext2[i];
        }
        /* 收敛检查 */
        double metric = 0.0;
        for (int i = 0; i < n; ++i) {
            metric += qAbs(extrinsic[i]);
        }
        metric /= static_cast<double>(n);
        result.iterations = iter + 1;
        m_stats.totalIterations++;
        emit decodeIteration(iter + 1, metric);
        if (qAbs(prevMetric - metric) < m_config.earlyStopThreshold && iter > 0) {
            result.converged = true;
            break;
        }
        prevMetric = metric;
    }
    /* 最终判决: 系统位 + 外信息 */
    QVector<double> finalLLR(n);
    for (int i = 0; i < n; ++i) {
        finalLLR[i] = systematic[i] + extrinsic[i];
    }
    result.llr = finalLLR;
    /* 硬判决 */
    result.decoded.resize((n + 7) / 8);
    result.decoded.fill(0);
    for (int i = 0; i < n; ++i) {
        if (finalLLR[i] < 0) {
            result.decoded[i / 8] |= static_cast<char>(1 << (7 - (i % 8)));
        }
    }
    result.success = true;
    result.finalMetric = prevMetric;
    /* 统计 */
    m_stats.totalDecodes++;
    m_stats.totalBitsDecoded += static_cast<quint64>(n);
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(m_stats.totalDecodes + m_stats.totalEncodes, 1ULL));
    emit decodeCompleted(result.decoded.size(), result.iterations);
    return result;
}

/** @brief RSC编码一步 @param state 当前状态 @param inputBit 输入比特 @return (新状态,校验输出) */
QPair<int, int> TurboCode2::rscEncode(int state, int inputBit) const
{
    /* 反馈 = input XOR parity(state, gen[0]) */
    int fbShiftreg = ((state << 1) | inputBit) & ((1 << m_config.constraintLength) - 1);
    int feedback = __builtin_parity(fbShiftreg & m_config.generators[0]);
    int newState = ((state << 1) | feedback) & m_mask;
    /* 校验输出 = parity(new_state, gen[1]) */
    int parityOut = __builtin_parity(((newState << 1) | feedback)
                    & m_config.generators[1]);
    return {newState, parityOut};
}

/** @brief RSC网格转移 @param fromState 起始状态 @param input 输入 @return (下一状态, 校验) */
QPair<int, int> TurboCode2::rscTransition(int fromState, int input) const
{
    return rscEncode(fromState, input);
}

/** @brief Log-MAP译码 @param sys 系统位LLR @param parity 校验位LLR @param extrinsic 外信息 @return (后验LLR, 新外信息) */
QPair<QVector<double>, QVector<double>> TurboCode2::decodeLogMAP(
    const QVector<double>& sys,
    const QVector<double>& parity,
    const QVector<double>& extrinsic) const
{
    int n = sys.size();
    int ns = m_numStates;
    /* 前向度量 alpha[time][state] (log域) */
    QVector<QVector<double>> alpha(n + 1, QVector<double>(ns, -1e30));
    alpha[0][0] = 0.0; /* 起始状态0 */
    /* 后向度量 beta[time][state] (log域) */
    QVector<QVector<double>> beta(n + 1, QVector<double>(ns, -1e30));
    beta[n][0] = 0.0; /* 终止状态0 */
    /* 分支度量 gamma[time][state][input] */
    /* 用 alpha + gamma 前向递推 */
    for (int t = 0; t < n; ++t) {
        double maxAlpha = -1e30;
        for (int s = 0; s < ns; ++s) {
            for (int inp = 0; inp <= 1; ++inp) {
                auto [nextS, pOut] = rscTransition(s, inp);
                /* 分支度量 = 0.5 * (inp * sys[t] + pOut * parity[t] + inp * extrinsic[t]) */
                double gamma = 0.5 * (static_cast<double>(inp) * sys[t]
                              + static_cast<double>(pOut) * parity[t]
                              + static_cast<double>(inp) * extrinsic[t]);
                alpha[t + 1][nextS] = logSumExp(alpha[t + 1][nextS],
                                                 alpha[t][s] + gamma);
            }
            if (alpha[t + 1][s] > maxAlpha) maxAlpha = alpha[t + 1][s];
        }
        /* 归一化(减最大值防止溢出) */
        for (int s = 0; s < ns; ++s) {
            alpha[t + 1][s] -= maxAlpha;
        }
    }
    /* 后向递推 */
    for (int t = n - 1; t >= 0; --t) {
        double maxBeta = -1e30;
        for (int s = 0; s < ns; ++s) {
            for (int inp = 0; inp <= 1; ++inp) {
                auto [nextS, pOut] = rscTransition(s, inp);
                double gamma = 0.5 * (static_cast<double>(inp) * sys[t]
                              + static_cast<double>(pOut) * parity[t]
                              + static_cast<double>(inp) * extrinsic[t]);
                beta[t][s] = logSumExp(beta[t][s],
                                       beta[t + 1][nextS] + gamma);
            }
            if (beta[t][s] > maxBeta) maxBeta = beta[t][s];
        }
        for (int s = 0; s < ns; ++s) {
            beta[t][s] -= maxBeta;
        }
    }
    /* 计算后验LLR和外信息 */
    QVector<double> app(n, 0.0);
    QVector<double> ext(n, 0.0);
    for (int t = 0; t < n; ++t) {
        double p1 = -1e30; /* 输入=1的后验 */
        double p0 = -1e30; /* 输入=0的后验 */
        for (int s = 0; s < ns; ++s) {
            for (int inp = 0; inp <= 1; ++inp) {
                auto [nextS, pOut] = rscTransition(s, inp);
                double gamma = 0.5 * (static_cast<double>(inp) * sys[t]
                              + static_cast<double>(pOut) * parity[t]
                              + static_cast<double>(inp) * extrinsic[t]);
                double metric = alpha[t][s] + gamma + beta[t + 1][nextS];
                if (inp == 1) {
                    p1 = logSumExp(p1, metric);
                } else {
                    p0 = logSumExp(p0, metric);
                }
            }
        }
        app[t] = p1 - p0;
        /* 外信息 = 后验 - 系统位 - 输入外信息 */
        ext[t] = app[t] - sys[t] - extrinsic[t];
    }
    return {app, ext};
}

/** @brief MAP译码(概率域) @param sys 系统位LLR @param parity 校验位LLR @param extrinsic 外信息 @return (后验LLR, 新外信息) */
QPair<QVector<double>, QVector<double>> TurboCode2::decodeMAP(
    const QVector<double>& sys,
    const QVector<double>& parity,
    const QVector<double>& extrinsic) const
{
    /* MAP在概率域计算，先转到Log-MAP的简化实现 */
    return decodeLogMAP(sys, parity, extrinsic);
}

/** @brief SOVA译码(软输出Viterbi) @param sys 系统位LLR @param parity 校验位LLR @param extrinsic 外信息 @return (后验LLR, 新外信息) */
QPair<QVector<double>, QVector<double>> TurboCode2::decodeSOVA(
    const QVector<double>& sys,
    const QVector<double>& parity,
    const QVector<double>& extrinsic) const
{
    int n = sys.size();
    int ns = m_numStates;
    /* 前向Viterbi */
    QVector<QVector<double>> pathMetric(n + 1, QVector<double>(ns, 1e30));
    QVector<QVector<int>> survivor(n + 1, QVector<int>(ns, 0));
    pathMetric[0][0] = 0.0;
    for (int t = 0; t < n; ++t) {
        for (int s = 0; s < ns; ++s) {
            for (int inp = 0; inp <= 1; ++inp) {
                auto [nextS, pOut] = rscTransition(s, inp);
                double branch = qAbs(sys[t] * (2 * inp - 1)
                          + parity[t] * (2 * pOut - 1)
                          + extrinsic[t] * (2 * inp - 1));
                double newMetric = pathMetric[t][s] + branch;
                if (newMetric < pathMetric[t + 1][nextS]) {
                    pathMetric[t + 1][nextS] = newMetric;
                    survivor[t + 1][nextS] = s;
                }
            }
        }
    }
    /* 回溯 */
    QVector<int> decoded(n, 0);
    int bestState = 0;
    double bestMetric = pathMetric[n][0];
    for (int s = 1; s < ns; ++s) {
        if (pathMetric[n][s] < bestMetric) {
            bestMetric = pathMetric[n][s];
            bestState = s;
        }
    }
    int currentState = bestState;
    for (int t = n - 1; t >= 0; --t) {
        int prevState = survivor[t + 1][currentState];
        /* 确定输入比特 */
        decoded[t] = (currentState > prevState ||
                     currentState == ((prevState << 1) & m_mask)) ? 1 : 0;
        currentState = prevState;
    }
    /* 软输出近似: 硬判决 * max(|LLR|) */
    QVector<double> app(n), ext(n);
    for (int t = 0; t < n; ++t) {
        double hardVal = decoded[t] == 1 ? 1.0 : -1.0;
        app[t] = hardVal * qMax(qAbs(sys[t]), 0.5);
        ext[t] = app[t] - sys[t] - extrinsic[t];
    }
    return {app, ext};
}

/** @brief 生成伪随机交织器 @param size 大小 @return 交织索引表 */
QVector<int> TurboCode2::generateInterleaver(int size) const
{
    QVector<int> table(size);
    std::iota(table.begin(), table.end(), 0);
    /* 基于S-random准则的交织器 */
    int s = static_cast<int>(qSqrt(static_cast<double>(2 * size)));
    std::mt19937 rng(42); /* 固定种子保证可复现 */
    for (int attempts = 0; attempts < size * 10; ++attempts) {
        std::shuffle(table.begin(), table.end(), rng);
        bool valid = true;
        for (int i = 0; i < size && valid; ++i) {
            for (int j = qMax(0, i - s); j < i; ++j) {
                if (qAbs(table[i] - table[j]) < s) {
                    valid = false;
                    break;
                }
            }
        }
        if (valid) break;
    }
    return table;
}

/** @brief log(sum(exp(a),exp(b))) 数值稳定计算 */
double TurboCode2::logSumExp(double a, double b) const
{
    if (a == -1e30) return b;
    if (b == -1e30) return a;
    double maxVal = qMax(a, b);
    return maxVal + qLn(qExp(a - maxVal) + qExp(b - maxVal));
}

/** @brief 获取交织索引 @return 交织映射表 */
QVector<int> TurboCode2::interleaver() const { return m_interleaver; }
/** @brief 获取编码速率 @return 速率 */
int TurboCode2::rate() const { return 3; } /* rate 1/3 */
/** @brief 获取统计 @return 统计 */
TurboCode2::Stats TurboCode2::stats() const { return m_stats; }
/** @brief 重置统计 */
void TurboCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

