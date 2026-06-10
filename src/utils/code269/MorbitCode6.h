/**
 * @file MorbitCode6.h
 * @brief Morbit密码(3x3网格符号映射与单字母密钥驱动置换紧凑视觉编码) — Morbit Cipher with 3x3 Grid Symbol Mapping and Monoalphabetic Key-Driven Permutation for Compact Visual Encoding
 *
 * 功能: 实现Morbit密码(Morbit cipher)，采用3x3网格符号映射(3x3 grid symbol mapping)
 *       与单字母密钥驱动置换(monoalphabetic key-driven permutation)实现紧凑视觉编码(compact visual encoding)。
 *
 * 协作: BaconCode5(培根密码) / PlayfairCipher4(Playfair密码) / SubstitutionCipher3(替换密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Morbit密码(3x3网格符号映射与单字母密钥驱动置换紧凑视觉编码)
 */
class MorbitCode6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        int gridSize = 3;
        double avgProcessingTimeMs = 0.0;
    };

    explicit MorbitCode6(QObject *parent = nullptr);
    ~MorbitCode6() override;

    /** @brief Set the 9-character permutation key */
    void setKey(const QString& key);

    /** @brief Set grid symbols (defaults to "123456789") */
    void setGridSymbols(const QString& symbols);

    /** @brief Encode plaintext to Morbit cipher text */
    QString encode(const QString& plaintext) const;

    /** @brief Decode Morbit cipher text to plaintext */
    QString decode(const QString& ciphertext) const;

    /** @brief Get the current permutation mapping */
    QVector<int> permutationMap() const;

    /** @brief Get the inverse permutation mapping */
    QVector<int> inverseMap() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void codingCompleted(int inputLen, int outputLen, double timeMs);

private:
    QString m_key;
    QString m_gridSymbols = QStringLiteral("123456789");
    QVector<int> m_forward;     // forward permutation [0..8] -> permuted
    QVector<int> m_inverse;     // inverse permutation

    mutable Stats m_stats;
    mutable double m_timeSum = 0.0;

    /** @brief Build permutation from key */
    void buildPermutation();

    /** @brief Map digit pairs through grid */
    QString mapPair(int r, int c, bool encode) const;

    /** @brief Validate key */
    bool isValidKey(const QString& key) const;
};
