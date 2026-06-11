/**
 * @file BazeleriesCode8.h
 * @brief Bazeleries密码(交错关键字转置与旋转矩形网格实现嵌套双层加密) — Bazeleries Cipher with Interleaved Keyword Transposition and Rotated Rectangular Grid for Nested Double-Layer Encryption
 *
 * 功能: 实现Bazeleries密码(Bazeleries cipher)，采用交错关键字转置(interleaved keyword transposition)
 *       与旋转矩形网格(rotated rectangular grid)实现嵌套双层加密(nested double-layer encryption)。
 *
 * 协作: CaesarCipher(凯撒密码) / VigenereCipher(维吉尼亚密码) / TranspositionCipher(转置密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

class BazeleriesCode8 : public QObject {
    Q_OBJECT

public:
    /** @brief Grid state for the rectangular transposition layer */
    struct GridState {
        int rows = 0;
        int cols = 0;
        int rotation = 0;    // grid rotation angle (0, 90, 180, 270)
        QVector<QVector<QChar>> cells;
    };

    /** @brief Cipher result */
    struct CipherResult {
        QString output;
        int gridRows = 0;
        int gridCols = 0;
        int rotations = 0;
        bool success = false;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalEncryptions = 0;
        quint64 totalDecryptions = 0;
        int lastInputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BazeleriesCode8(QObject *parent = nullptr);
    ~BazeleriesCode8() override;

    void setKeyword(const QString& keyword);
    void setGridRotation(int degrees);

    /** @brief Encrypt plaintext through nested double-layer Bazeleries cipher */
    CipherResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext through nested double-layer Bazeleries cipher */
    CipherResult decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptDone(int length, double timeMs);
    void decryptDone(int length, double timeMs);

private:
    QString m_keyword;
    int m_rotation = 0;
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Generate keyword-based permutation order */
    QVector<int> keywordPermutation(const QString& keyword) const;

    /** @brief Layer 1: Interleaved keyword columnar transposition */
    QString keywordTranspose(const QString& text, const QVector<int>& perm, bool encrypt) const;

    /** @brief Fill rotated rectangular grid from text */
    GridState fillGrid(const QString& text, int rows, int cols) const;

    /** @brief Read text from rotated grid */
    QString readGrid(const GridState& grid) const;

    /** @brief Layer 2: Rotated rectangular grid transposition */
    QString gridTranspose(const QString& text, bool encrypt) const;

    /** @brief Compute grid dimensions for given text length */
    void computeGridDims(int length, int& rows, int& cols) const;
};
