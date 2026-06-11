/**
 * @file PolluxCode8.h
 * @brief Pollux密码(可配置点划莫尔斯映射与变长分隔符的灵活电报加密) — Pollux Cipher with Configurable Dot-dash Morse Mapping and Variable-length Delimiter for Flexible Telegraph Encryption
 *
 * 功能: 实现Pollux密码(Pollux cipher)，采用可配置点划莫尔斯映射(configurable dot-dash Morse mapping)
 *       与变长分隔符(variable-length delimiter)实现灵活电报加密(flexible telegraph encryption)。
 *
 * 协作: CaesarCipher5(凯撒密码) / VigenereCipher6(维吉尼亚密码) / MorseCode7(莫尔斯码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QMap>

/**
 * @brief Pollux密码(可配置点划莫尔斯映射与变长分隔符的灵活电报加密)
 */
class PolluxCode8 : public QObject {
    Q_OBJECT

public:
    /** @brief Morse element type */
    enum MorseElement { Dot = 0, Dash = 1, Separator = 2 };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int lastInputLen = 0;
        int lastCipherLen = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PolluxCode8(QObject *parent = nullptr);
    ~PolluxCode8() override;

    /** @brief Set digit-to-morse mapping (digit 0-9 -> MorseElement) */
    void setMapping(const QMap<int, MorseElement>& mapping);

    /** @brief Set letter delimiter string for Morse output */
    void setDelimiter(const QString& delim);

    /** @brief Set word separator for Morse output */
    void setWordSeparator(const QString& sep);

    /** @brief Encrypt plaintext to Pollux cipher digits */
    QString encrypt(const QString& plaintext) const;

    /** @brief Decrypt Pollux cipher digits back to plaintext */
    QString decrypt(const QString& ciphertext) const;

    /** @brief Convert text to Morse code string */
    QString toMorse(const QString& text) const;

    /** @brief Convert Morse code string back to text */
    QString fromMorse(const QString& morse) const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void operationDone(const QString& op, int inLen, int outLen, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Digit -> Morse element mapping */
    QMap<int, MorseElement> m_digitMap;

    /** @brief Morse element -> list of digits (reverse mapping) */
    QMap<MorseElement, QVector<int>> m_reverseMap;

    QString m_delimiter = QStringLiteral(" ");
    QString m_wordSeparator = QStringLiteral("   ");

    /** @brief Internal Morse lookup table (A-Z, 0-9) */
    QMap<QChar, QString> m_morseTable;

    /** @brief Reverse Morse lookup (morse -> char) */
    QMap<QString, QChar> m_reverseMorse;

    void initMorseTable();
    void initDefaultMapping();
    void rebuildReverseMap();
};
