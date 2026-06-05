/**
 * @file MuLawEncoder.cpp
 * @brief μ-law/A-law压扩编解码器实现
 */

#include "MuLawEncoder.h"

#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

// ═══════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════

MuLawEncoder::MuLawEncoder(QObject* parent) : QObject(parent) {}
MuLawEncoder::~MuLawEncoder() = default;

void MuLawEncoder::setLaw(Law law) { m_law = law; }
MuLawEncoder::Law MuLawEncoder::law() const { return m_law; }

// ═══════════════════════════════════════════════════════════
// 采样级编解码
// ═══════════════════════════════════════════════════════════

quint8 MuLawEncoder::encodeSample(qint16 sample)
{
    return (m_law == MuLaw) ? muLawEncode(sample) : aLawEncode(sample);
}

qint16 MuLawEncoder::decodeSample(quint8 code)
{
    return (m_law == MuLaw) ? muLawDecode(code) : aLawDecode(code);
}

// ═══════════════════════════════════════════════════════════
// 批量编解码
// ═══════════════════════════════════════════════════════════

QByteArray MuLawEncoder::encode(const QByteArray& pcmData)
{
    if (pcmData.size() < 2) {
        emit error(tr("μ-law编码错误: PCM数据不足"));
        return {};
    }

    QElapsedTimer timer;
    timer.start();

    const int sampleCount = pcmData.size() / 2;
    QByteArray result;
    result.reserve(sampleCount + 4);

    // 头部: 采样数 + 压扩标准
    result.append(static_cast<char>((sampleCount >> 24) & 0xFF));
    result.append(static_cast<char>((sampleCount >> 16) & 0xFF));
    result.append(static_cast<char>((sampleCount >> 8) & 0xFF));
    result.append(static_cast<char>(sampleCount & 0xFF));
    result.append(static_cast<char>(m_law));

    double snrSum = 0.0;
    for (int i = 0; i < sampleCount; ++i) {
        qint16 sample = static_cast<qint16>(
            (static_cast<quint8>(pcmData[i * 2 + 1]) << 8) |
             static_cast<quint8>(pcmData[i * 2]));
        quint8 code = encodeSample(sample);
        result.append(static_cast<char>(code));

        // 计算信噪比
        qint16 reconstructed = decodeSample(code);
        snrSum += computeSnr(sample, reconstructed);
    }

    m_stats.totalEncodes++;
    m_stats.totalSamples += static_cast<quint64>(sampleCount);

    const double avgSnr = snrSum / static_cast<double>(std::max(sampleCount, 1));
    if (m_stats.totalEncodes == 1) {
        m_stats.avgSnr = avgSnr;
    } else {
        m_stats.avgSnr += (avgSnr - m_stats.avgSnr) /
                          static_cast<double>(m_stats.totalEncodes);
    }

    emit encoded(sampleCount, 0.5); // 16bit -> 8bit = 2:1压缩
    return result;
}

QByteArray MuLawEncoder::decode(const QByteArray& compandedData)
{
    if (compandedData.size() < 5) {
        emit error(tr("μ-law解码错误: 数据不足"));
        return {};
    }

    // 读取头部
    const int sampleCount = (static_cast<quint8>(compandedData[0]) << 24) |
                            (static_cast<quint8>(compandedData[1]) << 16) |
                            (static_cast<quint8>(compandedData[2]) << 8) |
                             static_cast<quint8>(compandedData[3]);

    QByteArray result;
    result.reserve(sampleCount * 2);

    for (int i = 0; i < sampleCount && (5 + i) < compandedData.size(); ++i) {
        quint8 code = static_cast<quint8>(compandedData[5 + i]);
        qint16 sample = decodeSample(code);
        result.append(static_cast<char>(sample & 0xFF));
        result.append(static_cast<char>((sample >> 8) & 0xFF));
    }

    m_stats.totalDecodes++;
    m_stats.totalSamples += static_cast<quint64>(sampleCount);

    emit decoded(sampleCount);
    return result;
}

// ═══════════════════════════════════════════════════════════
// 统计
// ═══════════════════════════════════════════════════════════

MuLawEncoder::Stats MuLawEncoder::stats() const { return m_stats; }
void MuLawEncoder::resetStatistics() { m_stats = Stats{}; }

// ═══════════════════════════════════════════════════════════
// μ-law编解码
// ═══════════════════════════════════════════════════════════

quint8 MuLawEncoder::muLawEncode(qint16 sample)
{
    // μ-law: μ=255, 14位输入 → 8位输出
    const int BIAS = 0x84;
    const int CLIP = 32635;

    int sign = (sample < 0) ? 0 : 1;
    int magnitude = std::abs(sample);
    if (magnitude > CLIP) magnitude = CLIP;
    magnitude += BIAS;

    int exponent = 7;
    for (int exp = 7; exp > 0; --exp) {
        if (magnitude & (1 << (exp + 6))) {
            exponent = exp;
            break;
        }
    }

    int mantissa = (magnitude >> (exponent + 3)) & 0x0F;
    return static_cast<quint8>(~((sign << 7) | (exponent << 4) | mantissa));
}

qint16 MuLawEncoder::muLawDecode(quint8 code)
{
    code = ~code;

    int sign = (code >> 7) & 1;
    int exponent = (code >> 4) & 0x07;
    int mantissa = code & 0x0F;

    int magnitude = ((mantissa << 3) | 0x84) << exponent;
    magnitude -= 0x84;

    return static_cast<qint16>(sign ? magnitude : -magnitude);
}

// ═══════════════════════════════════════════════════════════
// A-law编解码
// ═══════════════════════════════════════════════════════════

quint8 MuLawEncoder::aLawEncode(qint16 sample)
{
    const int CLIP = 31744;

    int sign = (sample < 0) ? 0 : 1;
    int magnitude = std::abs(sample);
    if (magnitude > CLIP) magnitude = CLIP;

    int exponent = 0;
    if (magnitude >= 2048) {
        exponent = 7;
        for (int e = 7; e > 0; --e) {
            if (magnitude & (1 << (e + 10))) {
                exponent = e;
                break;
            }
        }
    }

    int mantissa;
    if (exponent == 0) {
        mantissa = (magnitude >> 4) & 0x0F;
    } else {
        mantissa = (magnitude >> (exponent + 3)) & 0x0F;
    }

    quint8 code = static_cast<quint8>((sign << 7) | (exponent << 4) | mantissa);
    return static_cast<quint8>(code ^ 0x55);
}

qint16 MuLawEncoder::aLawDecode(quint8 code)
{
    code ^= 0x55;

    int sign = (code >> 7) & 1;
    int exponent = (code >> 4) & 0x07;
    int mantissa = code & 0x0F;

    int magnitude;
    if (exponent == 0) {
        magnitude = (mantissa << 4) | 0x08;
    } else {
        magnitude = ((mantissa << 4) | 0x108) << (exponent - 1);
    }

    return static_cast<qint16>(sign ? magnitude : -magnitude);
}

// ═══════════════════════════════════════════════════════════
// 信噪比计算
// ═══════════════════════════════════════════════════════════

double MuLawEncoder::computeSnr(qint16 original, qint16 reconstructed)
{
    const double diff = static_cast<double>(original) - static_cast<double>(reconstructed);
    const double signal = static_cast<double>(original) * static_cast<double>(original);
    const double noise = diff * diff;
    if (noise < 1e-10) return 96.0; // 最大SNR
    return 10.0 * std::log10(signal / noise);
}
