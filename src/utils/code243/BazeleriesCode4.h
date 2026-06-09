/**
 * @file BazeleriesCode4.h
 * @brief Bazeleries密码(扰乱列置换+关键字不规则列读取顺序) — Bazeleries Cipher with Disrupted Columnar Transposition and Keyword-Derived Irregular Column Reading Order
 *
 * 功能: 实现Bazeleries密码(Bazeleries cipher)，结合扰乱列置换转置(disrupted
 *       columnar transposition)和由关键字派生的不规则列读取顺序(keyword-derived
 *       irregular column reading order)进行加密解密。
 *
 * 协作: PlayfairCipher3(Playfair) / VigenereCipher5(Vigenere) / HillCipher6(Hill)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief Bazeleries密码(扰乱列置换+关键字不规则列读取顺序)
 */
class BazeleriesCode4 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncrypts = 0;
        int numDecrypts = 0;
        int keywordLength = 0;
        int numColumns = 0;
        int inputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit BazeleriesCode4(QObject *parent = nullptr);
    ~BazeleriesCode4() override;

    /** @brief Set keyword for column order derivation */
    void setKeyword(const QString& keyword);

    /** @brief Set number of disruption rows (0 = auto) */
    void setDisruptionRows(int rows);

    /** @brief Encrypt plaintext using Bazeleries cipher */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext using Bazeleries cipher */
    QString decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationCompleted(const QString& op, int length, double timeMs);

private:
    QString m_keyword;
    int m_disruptionRows = 0;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Derive column reading order from keyword */
    QVector<int> deriveColumnOrder(const QString& keyword) const;

    /** @brief Inverse column order (for decryption) */
    QVector<int> inverseOrder(const QVector<int>& order) const;

    /** @brief Fill transposition grid with disruption pattern */
    QVector<QVector<QChar>> fillGrid(const QString& text, int cols, int rows) const;

    /** @brief Read grid columns in given order */
    QString readColumns(const QVector<QVector<QChar>>& grid, const QVector<int>& order) const;

    /** @brief Write ciphertext into grid columns in given order for decryption */
    QVector<QVector<QChar>> writeColumns(const QString& text, int cols, int rows,
                                          const QVector<int>& order) const;

    /** @brief Read grid row-wise */
    QString readRows(const QVector<QVector<QChar>>& grid) const;
};
