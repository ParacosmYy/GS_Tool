/**
 * @file DigrafidCode5.h
 * @brief Digrafid密码(Bifid坐标拆分+关键字周期矩形转置) — Digrafid Cipher with Bifid-Style Coordinate Splitting and Rectangular Transposition with Keyword-Derived Period
 *
 * 功能: 实现Digrafid密码(Digrafid cipher)，结合Bifid风格坐标拆分
 *       (Bifid-style coordinate splitting)将字母对映射为三元坐标，
 *       关键字派生周期(keyword-derived period)的矩形转置(rectangular
 *       transposition)实现双图表加密。
 *
 * 协作: BifidCipher4(Bifid密码) / PlayfairCipher3(Playfair密码) / TrifidCipher3(Trifid)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QChar>

/**
 * @brief Digrafid密码(Bifid坐标拆分+矩形转置)
 */
class DigrafidCode5 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int numEncryptions = 0;
        int numDecryptions = 0;
        int keyLength = 0;
        int period = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit DigrafidCode5(QObject *parent = nullptr);
    ~DigrafidCode5() override;

    /** @brief Set keyword for grid permutation */
    void setKeyword(const QString& keyword);

    /** @brief Set transposition period (0 = full message) */
    void setPeriod(int period);

    /** @brief Encrypt plaintext */
    QString encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    QString decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void encryptionCompleted(int inputLen, int outputLen, double timeMs);
    void decryptionCompleted(int inputLen, int outputLen, double timeMs);

private:
    QString m_keyword;
    int m_period = 0;           // 0 = full message period
    QVector<QChar> m_grid;      // 6x6 grid (36 chars: A-Z + 0-9)
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build the 6x6 substitution grid from keyword */
    void buildGrid();

    /** @brief Find coordinates (row, col) of a character in the grid */
    bool findCoord(QChar ch, int& row, int& col) const;

    /** @brief Get character at grid position */
    QChar gridChar(int row, int col) const;

    /** @brief Normalize text: uppercase, strip non-alphanumeric */
    QString normalize(const QString& text) const;

    /** @brief Core Bifid-style coordinate split and recombine */
    QString processPairs(const QString& text, bool encrypt) const;

    /** @brief Rectangular transposition by keyword-derived period */
    QVector<int> applyTransposition(const QVector<int>& indices,
                                    int width, bool encrypt) const;
};
