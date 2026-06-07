/**
 * @file Rope6.h
 * @brief Rope数据结构(Piece Table文本编辑+操作日志撤销/重做+UTF-8码点导航) — Rope with Piece Table for Text Editing, Undo/Redo via Operation Log and UTF-8 Codepoint Navigation
 *
 * 功能: 实现Rope数据结构，支持Piece Table文本编辑、
 *       操作日志撤销/重做和UTF-8码点级导航。
 *
 * 协作: SuffixTree6(后缀树) / Rope5(Rope) / Trie8(字典树)
 */
#pragma once

#include <QObject>
#include <QVector>
#include <QString>
#include <QByteArray>

/**
 * @brief Rope数据结构(Piece Table文本编辑+操作日志撤销/重做+UTF-8码点导航)
 */
class Rope6 : public QObject {
    Q_OBJECT

public:
    /** @brief Running statistics */
    struct Stats {
        quint64 totalOps = 0;
        int textLength = 0;
        int numPieces = 0;
        double avgProcessingTimeMs = 0.0;
    };

    explicit Rope6(QObject *parent = nullptr);
    ~Rope6() override;

    /** @brief Initialize rope with initial text */
    void setText(const QString& text);

    /** @brief Insert text at codepoint position */
    void insert(int pos, const QString& text);

    /** @brief Delete text at codepoint position */
    void remove(int pos, int length);

    /** @brief Get full text */
    QString text() const;

    /** @brief Get substring at codepoint range */
    QString substring(int start, int length) const;

    /** @brief Navigate to Nth UTF-8 codepoint, return byte offset */
    int codepointToByteOffset(int codepointIndex) const;

    /** @brief Count total codepoints */
    int codepointCount() const;

    /** @brief Undo last operation */
    bool undo();

    /** @brief Redo last undone operation */
    bool redo();

    /** @brief Check if undo is available */
    bool canUndo() const;

    /** @brief Check if redo is available */
    bool canRedo() const;

    const Stats& stats() const { return m_stats; }
    void resetStatistics();

signals:
    void textChanged(int newLength);
    void operationCompleted(const QString& op, double timeMs);

private:
    /** @brief Piece descriptor: references a buffer span */
    struct Piece {
        int bufferIndex = 0; // 0=original, 1=add
        int start = 0;
        int length = 0;
    };

    /** @brief Operation log entry for undo/redo */
    struct OpEntry {
        enum Type { Insert, Remove };
        Type type = Insert;
        int position = 0;
        QString text;
        int pieceSnapshotSize = 0; // for restoring piece count
    };

    QByteArray m_originalBuffer; // immutable original text
    QByteArray m_addBuffer;      // append-only addition buffer
    QVector<Piece> m_pieces;     // piece table
    int m_codepointCount = 0;

    // Undo/redo stacks
    QVector<OpEntry> m_undoStack;
    QVector<OpEntry> m_redoStack;

    Stats m_stats;
    double m_timeSum = 0.0;

    /** @brief Rebuild text from pieces into a QByteArray */
    QByteArray buildBytes() const;

    /** @brief Update cached codepoint count */
    void updateCodepointCount();

    /** @brief Find piece and offset for a given codepoint position */
    QPair<int, int> findPieceForPosition(int codepointPos) const;
};
