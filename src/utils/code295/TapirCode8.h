/**
 * @file TapirCode8.h
 * @brief Tapir编码(变长符号编码与位置相关XOR扩散实现轻量级流密码) — Tapir Code with Variable-length Symbol Encoding and Position-dependent XOR Diffusion for Lightweight Stream Cipher
 *
 * 功能: 实现Tapir编码(Tapir code)，采用变长符号编码(variable-length symbol encoding)
 *       与位置相关XOR扩散(position-dependent XOR diffusion)实现轻量级流密码(lightweight stream cipher)。
 *
 * 协作: BCHCode9(BCH纠错码) / ReedSolomon14(Reed-Solomon码) / ConvolutionalCode7(卷积码)
 */
#pragma once

#include <QObject>
#include <QVector>

class TapirCode8 : public QObject {
    Q_OBJECT

public:
    /** @brief Encoding result */
    struct EncodeResult {
        QVector<quint8> encoded;
        int originalSize = 0;
        int encodedSize = 0;
        double compressionRatio = 0.0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalEncodes = 0;
        quint64 totalDecodes = 0;
        int lastBlockSize = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit TapirCode8(QObject *parent = nullptr);
    ~TapirCode8() override;

    /** @brief Set cipher key seed */
    void setKey(quint64 key);

    /** @brief Encode (encrypt) data with Tapir stream cipher */
    EncodeResult encode(const QVector<quint8>& data);

    /** @brief Decode (decrypt) data with Tapir stream cipher */
    QVector<quint8> decode(const QVector<quint8>& encoded, int originalSize);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encodeDone(int size, double ratio, double timeMs);
    void decodeDone(int size, double timeMs);

private:
    quint64 m_key = 0x5F3759DF;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Generate position-dependent XOR keystream byte */
    quint8 keystreamByte(int position) const;

    /** @brief Variable-length encode a symbol (4-bit or 8-bit) */
    QVector<quint8> varLengthEncode(const QVector<quint8>& data) const;

    /** @brief Variable-length decode */
    QVector<quint8> varLengthDecode(const QVector<quint8>& encoded,
                                     int originalSize) const;

    /** @brief Apply XOR diffusion layer */
    void xorDiffusion(QVector<quint8>& block, int basePos) const;

    /** @brief Reverse XOR diffusion layer */
    void xorDiffusionInverse(QVector<quint8>& block, int basePos) const;
};
