/**
 * @file StraddlingCheckerboard2.h
 * @brief 跨棋盘密码(优化棋盘布局+助记密钥调度) — Straddling Checkerboard Cipher with Optimized Layout and Mnemonic Key Scheduling
 *
 * 功能: 实现跨棋盘密码，支持优化棋盘布局生成、
 *       助记密钥调度和混合加密/解密。
 *
 * 协作: Playfair2(Playfair密码) / Vigenere3(Vigenere密码) / EnigmaRotor4(转子密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QPair>
#include <QString>

/**
 * @brief 跨棋盘密码(优化棋盘布局+助记密钥调度)
 */
class StraddlingCheckerboard2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit StraddlingCheckerboard2(QObject *parent = nullptr);
    ~StraddlingCheckerboard2() override;

    /** @brief Set the keyword for mnemonic key scheduling */
    void setKeyword(const QString& keyword);

    /** @brief Set blank row positions (two digits 0-9) */
    void setBlankRows(int row1, int row2);

    /** @brief Encrypt plaintext to digit string */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt digit string to plaintext */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Generate optimized checkerboard layout from keyword */
    QVector<QVector<QChar>> generateLayout() const;

    /** @brief Mnemonic key scheduling: derive permutation from keyword */
    QVector<int> mnemonicSchedule() const;

    /** @brief Get current layout */
    QVector<QVector<QChar>> getLayout() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherCompleted(const QString& op, int inLen, int outLen, double timeMs);

private:
    QString m_keyword;
    int m_blankRow1 = 2;
    int m_blankRow2 = 6;

    /** @brief 3x10 checkerboard layout [row][col] */
    QVector<QVector<QChar>> m_layout;

    /** @brief Reverse lookup: char -> (row, col) */
    QMap<QChar, QPair<int, int>> m_reverseMap;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Build checkerboard layout and reverse map */
    void buildLayout();

    /** @brief Alphabet string used for filling the board */
    static const QString ALPHABET;

    /** @brief Strip non-alpha characters and uppercase */
    static QString sanitize(const QString& text);
};
