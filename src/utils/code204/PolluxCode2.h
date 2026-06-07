/**
 * @file PolluxCode2.h
 * @brief Pollux密码变体(摩尔斯码填充分析+差分点划频率攻击) — Pollux Cipher Variant with Morse Code Padding Analysis and Differential Dot-Dash Frequency Attack
 *
 * 功能: 实现Pollux密码变体分析，支持摩尔斯码填充检测、
 *       差分点划频率攻击和自动密钥恢复。
 *
 * 协作: FrequencyAnalyzer(频率分析) / HammingCode3(汉明码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QMap>
#include <QString>

/**
 * @brief Pollux密码变体(摩尔斯码填充分析+差分点划频率攻击)
 */
class PolluxCode2 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int messageLength = 0;
        double confidenceScore = 0.0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit PolluxCode2(QObject *parent = nullptr);
    ~PolluxCode2() override;

    /** @brief Encrypt plaintext to Pollux cipher digits */
    QString encrypt(const QString& plaintext, const QMap<QChar, QString>& morseTable,
                    const QVector<int>& keyMap) const;

    /** @brief Decrypt Pollux cipher digits using known key */
    QString decrypt(const QString& cipher, const QMap<QChar, QString>& morseTable,
                    const QVector<int>& keyMap) const;

    /** @brief Analyze padding patterns in cipher text */
    QVector<double> paddingAnalysis(const QString& cipher) const;

    /** @brief Differential dot-dash frequency attack to recover key */
    QVector<int> frequencyAttack(const QString& cipher,
                                  const QMap<QChar, QString>& morseTable) const;

    /** @brief Build standard Morse code table */
    static QMap<QChar, QString> standardMorseTable();

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void analysisCompleted(double confidence, double timeMs);

private:
    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Count dot/dash frequency per digit position */
    void countFrequencies(const QString& morse, QVector<double>& dotFreq,
                          QVector<double>& dashFreq) const;
};
