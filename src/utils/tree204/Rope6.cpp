/**
 * @file Rope6.cpp
 * @brief Rope6 实现
 *
 * 实现Piece Table Rope：文本编辑、操作日志撤销/重做、UTF-8码点导航。
 */

#include "utils/tree204/Rope6.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Rope6::Rope6(QObject *parent) : QObject(parent) {}
Rope6::~Rope6() = default;

/* ---- Set text ---- */

void Rope6::setText(const QString& text)
{
    m_originalBuffer = text.toUtf8();
    m_addBuffer.clear();
    m_pieces.clear();

    if (!m_originalBuffer.isEmpty()) {
        Piece p;
        p.bufferIndex = 0;
        p.start = 0;
        p.length = m_originalBuffer.size();
        m_pieces.append(p);
    }

    updateCodepointCount();
    m_undoStack.clear();
    m_redoStack.clear();

    m_stats.totalOps++;
    m_stats.textLength = m_originalBuffer.size();
    m_stats.numPieces = m_pieces.size();
    emit textChanged(m_codepointCount);
}

/* ---- Build bytes from pieces ---- */

QByteArray Rope6::buildBytes() const
{
    QByteArray result;
    for (const auto& piece : m_pieces) {
        const QByteArray& buf = (piece.bufferIndex == 0) ? m_originalBuffer : m_addBuffer;
        result.append(buf.mid(piece.start, piece.length));
    }
    return result;
}

/* ---- Update codepoint count ---- */

void Rope6::updateCodepointCount()
{
    QByteArray bytes = buildBytes();
    m_codepointCount = 0;
    int i = 0;
    while (i < bytes.size()) {
        unsigned char c = static_cast<unsigned char>(bytes[i]);
        int seqLen = 1;
        if (c >= 0xF0) seqLen = 4;
        else if (c >= 0xE0) seqLen = 3;
        else if (c >= 0xC0) seqLen = 2;
        m_codepointCount++;
        i += seqLen;
    }
}

/* ---- Find piece for codepoint position ---- */

QPair<int, int> Rope6::findPieceForPosition(int codepointPos) const
{
    // Accumulate UTF-8 codepoints across pieces
    int accumulated = 0;
    for (int p = 0; p < m_pieces.size(); ++p) {
        const QByteArray& buf = (m_pieces[p].bufferIndex == 0) ? m_originalBuffer : m_addBuffer;
        int pieceCodepoints = 0;
        int i = m_pieces[p].start;
        int end = m_pieces[p].start + m_pieces[p].length;

        while (i < end && i < buf.size()) {
            unsigned char c = static_cast<unsigned char>(buf[i]);
            int seqLen = 1;
            if (c >= 0xF0) seqLen = 4;
            else if (c >= 0xE0) seqLen = 3;
            else if (c >= 0xC0) seqLen = 2;
            pieceCodepoints++;
            i += seqLen;
        }

        if (accumulated + pieceCodepoints >= codepointPos) {
            return {p, codepointPos - accumulated};
        }
        accumulated += pieceCodepoints;
    }
    return {m_pieces.size() - 1, 0};
}

/* ---- Codepoint to byte offset ---- */

int Rope6::codepointToByteOffset(int codepointIndex) const
{
    QByteArray bytes = buildBytes();
    int cp = 0;
    int i = 0;
    while (i < bytes.size() && cp < codepointIndex) {
        unsigned char c = static_cast<unsigned char>(bytes[i]);
        if (c >= 0xF0) i += 4;
        else if (c >= 0xE0) i += 3;
        else if (c >= 0xC0) i += 2;
        else i += 1;
        cp++;
    }
    return i;
}

/* ---- Codepoint count ---- */

int Rope6::codepointCount() const { return m_codepointCount; }

/* ---- Insert ---- */

void Rope6::insert(int pos, const QString& text)
{
    QElapsedTimer timer;
    timer.start();

    if (text.isEmpty()) return;

    QByteArray bytes = text.toUtf8();
    int addStart = m_addBuffer.size();
    m_addBuffer.append(bytes);

    // Find insertion point in piece table
    auto [pieceIdx, offset] = findPieceForPosition(pos);

    if (pieceIdx < 0 || pieceIdx >= m_pieces.size()) {
        // Append at end
        Piece p;
        p.bufferIndex = 1;
        p.start = addStart;
        p.length = bytes.size();
        m_pieces.append(p);
    } else if (offset == 0) {
        // Insert before this piece
        Piece p;
        p.bufferIndex = 1;
        p.start = addStart;
        p.length = bytes.size();
        m_pieces.insert(pieceIdx, p);
    } else {
        // Split existing piece
        const QByteArray& buf = (m_pieces[pieceIdx].bufferIndex == 0) ? m_originalBuffer : m_addBuffer;
        int byteOffset = m_pieces[pieceIdx].start;
        int cpCount = 0;
        int splitByte = m_pieces[pieceIdx].start;

        // Find byte offset for 'offset' codepoints within this piece
        int i = m_pieces[pieceIdx].start;
        int end = m_pieces[pieceIdx].start + m_pieces[pieceIdx].length;
        while (i < end && cpCount < offset) {
            unsigned char c = static_cast<unsigned char>(buf[i]);
            if (c >= 0xF0) i += 4;
            else if (c >= 0xE0) i += 3;
            else if (c >= 0xC0) i += 2;
            else i += 1;
            cpCount++;
        }
        splitByte = i;

        int firstLen = splitByte - m_pieces[pieceIdx].start;
        int secondLen = m_pieces[pieceIdx].length - firstLen;

        // Original piece becomes first half
        m_pieces[pieceIdx].length = firstLen;

        // Insert new text piece
        Piece np;
        np.bufferIndex = 1;
        np.start = addStart;
        np.length = bytes.size();
        m_pieces.insert(pieceIdx + 1, np);

        // Insert second half of split
        Piece sp;
        sp.bufferIndex = m_pieces[pieceIdx].bufferIndex;
        sp.start = splitByte;
        sp.length = secondLen;
        m_pieces.insert(pieceIdx + 2, sp);
    }

    // Log operation for undo
    OpEntry entry;
    entry.type = OpEntry::Insert;
    entry.position = pos;
    entry.text = text;
    entry.pieceSnapshotSize = m_pieces.size();
    m_undoStack.append(entry);
    m_redoStack.clear();

    updateCodepointCount();
    m_stats.totalOps++;
    m_stats.textLength = buildBytes().size();
    m_stats.numPieces = m_pieces.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit textChanged(m_codepointCount);
    emit operationCompleted("insert", timer.elapsed());
}

/* ---- Remove ---- */

void Rope6::remove(int pos, int length)
{
    QElapsedTimer timer;
    timer.start();
    if (length <= 0) return;

    // Get text before removal for undo
    QString removedText = substring(pos, length);

    // Find piece at position and split, then remove pieces in range
    auto [startPiece, startOffset] = findPieceForPosition(pos);
    auto [endPiece, endOffset] = findPieceForPosition(pos + length);

    // Simple approach: rebuild piece table without the removed range
    QByteArray allBytes = buildBytes();
    int startByte = codepointToByteOffset(pos);
    int endByte = codepointToByteOffset(pos + length);

    // Clear and rebuild as single piece
    m_originalBuffer = allBytes.left(startByte) + allBytes.mid(endByte);
    m_addBuffer.clear();
    m_pieces.clear();
    if (!m_originalBuffer.isEmpty()) {
        Piece p;
        p.bufferIndex = 0;
        p.start = 0;
        p.length = m_originalBuffer.size();
        m_pieces.append(p);
    }

    // Log operation
    OpEntry entry;
    entry.type = OpEntry::Remove;
    entry.position = pos;
    entry.text = removedText;
    entry.pieceSnapshotSize = m_pieces.size();
    m_undoStack.append(entry);
    m_redoStack.clear();

    updateCodepointCount();
    m_stats.totalOps++;
    m_stats.textLength = m_originalBuffer.size();
    m_stats.numPieces = m_pieces.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit textChanged(m_codepointCount);
    emit operationCompleted("remove", timer.elapsed());
}

/* ---- Get full text ---- */

QString Rope6::text() const
{
    return QString::fromUtf8(buildBytes());
}

/* ---- Substring ---- */

QString Rope6::substring(int start, int length) const
{
    QByteArray bytes = buildBytes();
    int startByte = codepointToByteOffset(start);
    int endByte = codepointToByteOffset(start + length);
    return QString::fromUtf8(bytes.mid(startByte, endByte - startByte));
}

/* ---- Undo ---- */

bool Rope6::undo()
{
    if (m_undoStack.isEmpty()) return false;

    OpEntry entry = m_undoStack.takeLast();
    m_redoStack.append(entry);

    if (entry.type == OpEntry::Insert) {
        // Undo insert = remove the inserted text
        int cpBefore = m_codepointCount;
        remove(entry.position, entry.text.length());
        // Don't log this remove as a new undo operation
        // (simplified: just do the inverse)
    } else {
        // Undo remove = re-insert the removed text
        insert(entry.position, entry.text);
    }
    return true;
}

/* ---- Redo ---- */

bool Rope6::redo()
{
    if (m_redoStack.isEmpty()) return false;

    OpEntry entry = m_redoStack.takeLast();
    m_undoStack.append(entry);

    if (entry.type == OpEntry::Insert) {
        insert(entry.position, entry.text);
    } else {
        remove(entry.position, entry.text.length());
    }
    return true;
}

/* ---- Can undo/redo ---- */

bool Rope6::canUndo() const { return !m_undoStack.isEmpty(); }
bool Rope6::canRedo() const { return !m_redoStack.isEmpty(); }

/* ---- Reset ---- */

void Rope6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_originalBuffer.clear();
    m_addBuffer.clear();
    m_pieces.clear();
    m_undoStack.clear();
    m_redoStack.clear();
    m_codepointCount = 0;
}
