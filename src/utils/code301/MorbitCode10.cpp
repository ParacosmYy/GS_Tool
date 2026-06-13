/**
 * @file MorbitCode10.cpp
 * @brief MorbitCode10 实现
 *
 * 实现Morbit密码：同音莫尔斯映射与随机符号选择实现可手工操作的字段加密。
 */

#include "utils/code301/MorbitCode10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MorbitCode10::MorbitCode10(QObject *parent)
    : QObject(parent)
{
    buildMorseTable();
}

MorbitCode10::~MorbitCode10() = default;

/* ---- Build standard Morse lookup ---- */

void MorbitCode10::buildMorseTable()
{
    // Standard International Morse code
    m_charToMorse['A'] = ".-";   m_charToMorse['B'] = "-...";
    m_charToMorse['C'] = "-.-."; m_charToMorse['D'] = "-..";
    m_charToMorse['E'] = ".";    m_charToMorse['F'] = "..-.";
    m_charToMorse['G'] = "--.";  m_charToMorse['H'] = "....";
    m_charToMorse['I'] = "..";   m_charToMorse['J'] = ".---";
    m_charToMorse['K'] = "-.-";  m_charToMorse['L'] = ".-..";
    m_charToMorse['M'] = "--";   m_charToMorse['N'] = "-.";
    m_charToMorse['O'] = "---";  m_charToMorse['P'] = ".--.";
    m_charToMorse['Q'] = "--.-"; m_charToMorse['R'] = ".-.";
    m_charToMorse['S'] = "...";  m_charToMorse['T'] = "-";
    m_charToMorse['U'] = "..-";  m_charToMorse['V'] = "...-";
    m_charToMorse['W'] = ".--";  m_charToMorse['X'] = "-..-";
    m_charToMorse['Y'] = "-.--"; m_charToMorse['Z'] = "--..";
    m_charToMorse['0'] = "-----"; m_charToMorse['1'] = ".----";
    m_charToMorse['2'] = "..---"; m_charToMorse['3'] = "...--";
    m_charToMorse['4'] = "....-"; m_charToMorse['5'] = ".....";
    m_charToMorse['6'] = "-...."; m_charToMorse['7'] = "--...";
    m_charToMorse['8'] = "---.."; m_charToMorse['9'] = "----.";

    // Reverse lookup
    for (auto it = m_charToMorse.constBegin(); it != m_charToMorse.constEnd(); ++it)
        m_morseToChar[it.value()] = it.key();
}

/* ---- Build Morbit 3x3 grid from key ---- */

void MorbitCode10::buildMorbitGrid()
{
    m_morseGrid.clear();
    if (m_key.length() < 9) return;

    // Morbit grid: 3x3 where each cell maps a pair of Morse symbols to a digit
    // Grid layout: row 0 = .-, row 1 = .-, col 0 = ., col 1 = -, col 2 = separator
    // Key digits 1-9 rearrange the grid
    QString normalized = m_key.left(9);
    for (int i = 0; i < 9; ++i)
        m_morseGrid[normalized[i]] = i + 1;
}

/* ---- Set key ---- */

void MorbitCode10::setKey(const QString& key)
{
    m_key = key;
    buildMorbitGrid();
}

/* ---- Enable homophonic ---- */

void MorbitCode10::setHomophonicEnabled(bool enabled)
{
    m_homophonic = enabled;
}

/* ---- Select random homophonic variant ---- */

QString MorbitCode10::randomVariant(const QVector<QString>& variants) const
{
    if (variants.isEmpty()) return ".";
    int idx = qrand() % variants.size();
    return variants[idx];
}

/* ---- Morse pair to digit ---- */

QChar MorbitCode10::morsePairToDigit(QChar first, QChar second) const
{
    // Map pair combinations via Morbit grid
    // Grid: . = 0, - = 1, separator = 2
    int row = (first == '.') ? 0 : 1;
    int col = (second == '.') ? 0 : 1;

    // Combine with key-based mapping
    int gridPos = row * 3 + col;
    if (m_key.length() > gridPos)
        return m_key[gridPos];
    return QChar('1' + gridPos);
}

/* ---- Text to Morse ---- */

QString MorbitCode10::textToMorse(const QString& text) const
{
    QString result;
    for (int i = 0; i < text.length(); ++i) {
        QChar c = text[i].toUpper();
        if (m_charToMorse.contains(c)) {
            if (!result.isEmpty()) result += " ";  // Letter separator
            result += m_charToMorse[c];
        }
    }
    return result;
}

/* ---- Morse to digit pairs ---- */

QString MorbitCode10::morseToDigits(const QString& morse) const
{
    QString result;
    // Pair up Morse symbols: dot=., dash=-, space=separator
    QString symbols;
    for (int i = 0; i < morse.length(); ++i) {
        if (morse[i] == '.' || morse[i] == '-')
            symbols += morse[i];
        else if (morse[i] == ' ')
            symbols += '/';   // Separator
    }

    // Pad to even length
    if (symbols.length() % 2 != 0)
        symbols += '.';

    for (int i = 0; i < symbols.length(); i += 2) {
        QChar digit = morsePairToDigit(symbols[i], symbols[i + 1]);
        result += digit;
    }
    return result;
}

/* ---- Encrypt ---- */

MorbitCode10::EncryptResult MorbitCode10::encrypt(const QString& plaintext)
{
    QElapsedTimer timer;
    timer.start();

    EncryptResult result;

    // Step 1: Convert to Morse
    QString morse = textToMorse(plaintext);
    result.morseSequence = morse;

    // Step 2: Convert Morse to digit pairs via Morbit key
    if (m_key.isEmpty()) {
        // Default key
        setKey("123456789");
    }

    result.ciphertext = morseToDigits(morse);
    result.symbolsUsed = result.ciphertext.length();
    result.elapsedMs = timer.elapsed();

    m_stats.totalEncryptions++;
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEncryptions;

    emit encryptDone(plaintext.length(), result.elapsedMs);
    return result;
}

/* ---- Decrypt ---- */

MorbitCode10::DecryptResult MorbitCode10::decrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    DecryptResult result;

    // Step 1: Reverse Morbit grid mapping
    if (m_key.isEmpty()) setKey("123456789");

    // Build reverse grid: digit -> pair
    QVector<QPair<QChar, QChar>> digitToPair(10);
    for (int i = 0; i < 9 && i < m_key.length(); ++i) {
        int row = i / 3;
        int col = i % 3;
        QChar first = (row < 2) ? ((row == 0) ? '.' : '-') : '/';
        QChar second = (col < 2) ? ((col == 0) ? '.' : '-') : '/';
        digitToPair[m_key[i].digitValue()] = {first, second};
    }

    // Step 2: Convert digits back to Morse symbols
    QString morse;
    for (int i = 0; i < ciphertext.length(); ++i) {
        int digit = ciphertext[i].digitValue();
        if (digit >= 1 && digit <= 9) {
            auto [f, s] = digitToPair[digit];
            if (f == '/' || s == '/') morse += ' ';
            else { morse += f; morse += s; }
        } else {
            result.errors++;
        }
    }

    // Step 3: Morse to text
    QStringList letters = morse.split(' ', Qt::SkipEmptyParts);
    QString plain;
    for (const QString& code : letters) {
        if (m_morseToChar.contains(code))
            plain += m_morseToChar[code];
        else
            result.errors++;
    }

    result.plaintext = plain;
    result.success = (result.errors == 0);
    result.elapsedMs = timer.elapsed();

    m_stats.totalDecryptions++;
    m_timeSum += result.elapsedMs;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncryptions + m_stats.totalDecryptions);

    emit decryptDone(ciphertext.length(), result.success, result.elapsedMs);
    return result;
}

/* ---- Reset ---- */

void MorbitCode10::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
