/**
 * @file FoursquareCode14.h
 * @brief 四方密码(渐进Polybius网格变异与坐标交织演化表加密) — Foursquare Code with Progressive Polybius Grid Mutation and Coordinate Interleaving for Evolving Tableaux Encryption
 *
 * 功能: 实现四方密码(Foursquare cipher)，采用渐进Polybius网格变异(progressive Polybius grid mutation)
 *       与坐标交织(coordinate interleaving)实现演化表加密(evolving tableaux encryption)。
 *
 * 协作: Playfair7(Playfair密码) / Bifid5(Bifid密码) / Adfgvx6(ADFGVX密码)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QPair>

/**
 * @brief 四方密码(渐进Polybius网格变异与坐标交织演化表加密)
 */
class FoursquareCode14 : public QObject {
    Q_OBJECT

public:
    /** @brief Cipher configuration */
    struct Config {
        QString key1;
        QString key2;
        int gridMutationRounds = 3;   // Number of progressive mutation rounds
        bool interleaving = true;
    };

    /** @brief Encryption result */
    struct EncResult {
        QString cipherText;
        int gridMutationApplied = 0;
        int blockCount = 0;
    };

    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int inputLength = 0;
        int outputLength = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit FoursquareCode14(QObject *parent = nullptr);
    ~FoursquareCode14() override;

    void setConfig(const Config& cfg);

    /** @brief Encrypt plaintext with evolving tableaux */
    EncResult encrypt(const QString& plaintext);

    /** @brief Decrypt ciphertext */
    QString decrypt(const QString& ciphertext);

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void cipherDone(int inputLen, int outputLen, double timeMs);

private:
    Config m_config;
    Stats m_stats;
    double m_timeSum = 0.0;

    // Four 5x5 grids: TL, TR, BL, BR stored as flat 25-char vectors
    QVector<QVector<QChar>> m_grids;   // 4 grids of 25 chars each
    QVector<QChar> m_plainAlphabet;     // Standard 5x5 alphabet (J merged into I)

    /** @brief Build a 5x5 Polybius grid from a keyword */
    QVector<QChar> buildGrid(const QString& keyword) const;

    /** @brief Apply progressive mutation to a grid */
    void mutateGrid(QVector<QChar>& grid, int round);

    /** @brief Find (row, col) of char in a grid */
    QPair<int, int> findInGrid(const QVector<QChar>& grid, QChar ch) const;

    /** @brief Get char at (row, col) in a grid */
    QChar gridAt(const QVector<QChar>& grid, int row, int col) const;

    /** @brief Prepare text: upper-case, merge J->I, pad */
    QString prepareText(const QString& text) const;

    /** @brief Coordinate interleaving: extract row/col pairs and interleave */
    QString interleave(const QString& rowStr, const QString& colStr) const;

    /** @brief Reverse coordinate interleaving */
    QPair<QString, QString> deinterleave(const QString& combined) const;
};
