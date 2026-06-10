/**
 * @file PolluxCode7.cpp
 * @brief PolluxCode7 实现
 *
 * 实现Pollux密码：约束剪枝密钥空间与摩尔斯元素频率分析高效解密。
 */

#include "utils/code274/PolluxCode7.h"

#include <QElapsedTimer>
#include <QMap>
#include <algorithm>

/* ---- Construction / Destruction ---- */

PolluxCode7::PolluxCode7(QObject *parent)
    : QObject(parent)
{
    buildMorseTables();
}

PolluxCode7::~PolluxCode7() = default;

/* ---- Build Morse code tables ---- */

void PolluxCode7::buildMorseTables()
{
    // Standard International Morse code
    m_morseEncode.insert('A', ".-");
    m_morseEncode.insert('B', "-...");
    m_morseEncode.insert('C', "-.-.");
    m_morseEncode.insert('D', "-..");
    m_morseEncode.insert('E', ".");
    m_morseEncode.insert('F', "..-.");
    m_morseEncode.insert('G', "--.");
    m_morseEncode.insert('H', "....");
    m_morseEncode.insert('I', "..");
    m_morseEncode.insert('J', ".---");
    m_morseEncode.insert('K', "-.-");
    m_morseEncode.insert('L', ".-..");
    m_morseEncode.insert('M', "--");
    m_morseEncode.insert('N', "-.");
    m_morseEncode.insert('O', "---");
    m_morseEncode.insert('P', ".--.");
    m_morseEncode.insert('Q', "--.-");
    m_morseEncode.insert('R', ".-.");
    m_morseEncode.insert('S', "...");
    m_morseEncode.insert('T', "-");
    m_morseEncode.insert('U', "..-");
    m_morseEncode.insert('V', "...-");
    m_morseEncode.insert('W', ".--");
    m_morseEncode.insert('X', "-..-");
    m_morseEncode.insert('Y', "-.--");
    m_morseEncode.insert('Z', "--..");
    m_morseEncode.insert('0', "-----");
    m_morseEncode.insert('1', ".----");
    m_morseEncode.insert('2', "..---");
    m_morseEncode.insert('3', "...--");
    m_morseEncode.insert('4', "....-");
    m_morseEncode.insert('5', ".....");
    m_morseEncode.insert('6', "-....");
    m_morseEncode.insert('7', "--...");
    m_morseEncode.insert('8', "---..");
    m_morseEncode.insert('9', "----.");

    // Build reverse lookup
    for (auto it = m_morseEncode.begin(); it != m_morseEncode.end(); ++it)
        m_morseDecode.insert(it.value(), it.key());
}

/* ---- Text to Morse element sequence ---- */

QVector<PolluxCode7::MorseElement> PolluxCode7::textToMorse(const QString& text) const
{
    QVector<MorseElement> result;
    QString upper = text.toUpper();
    for (int i = 0; i < upper.size(); ++i) {
        QChar ch = upper[i];
        if (ch == ' ') {
            result.append(WordSep);
            continue;
        }
        if (m_morseEncode.contains(ch)) {
            QString morse = m_morseEncode[ch];
            for (int j = 0; j < morse.size(); ++j) {
                result.append(morse[j] == '.' ? Dot : Dash);
            }
            if (i + 1 < upper.size() && upper[i + 1] != ' ')
                result.append(LetterSep);
        }
    }
    return result;
}

/* ---- Morse element sequence to text ---- */

QString PolluxCode7::morseToText(const QVector<MorseElement>& elements) const
{
    QString result;
    QString current;
    for (int i = 0; i < elements.size(); ++i) {
        if (elements[i] == Dot) current += '.';
        else if (elements[i] == Dash) current += '-';
        else if (elements[i] == LetterSep || elements[i] == WordSep) {
            if (m_morseDecode.contains(current))
                result += m_morseDecode[current];
            current.clear();
            if (elements[i] == WordSep) result += ' ';
        }
    }
    // Flush remaining
    if (!current.isEmpty() && m_morseDecode.contains(current))
        result += m_morseDecode[current];
    return result;
}

/* ---- Encrypt with known key ---- */

QString PolluxCode7::encrypt(const QString& plaintext, const QVector<int>& key)
{
    // Key: key[digit] → MorseElement (0=dot, 1=dash, 2=letterSep, 3=wordSep)
    QVector<MorseElement> morse = textToMorse(plaintext);
    QString result;
    for (int i = 0; i < morse.size(); ++i) {
        // Find key index that maps to this element
        for (int d = 0; d < qMin(10, key.size()); ++d) {
            if (key[d] == static_cast<int>(morse[i])) {
                result += QString::number(d);
                break;
            }
        }
    }
    return result;
}

/* ---- Decrypt with known key ---- */

QString PolluxCode7::decrypt(const QString& ciphertext, const QVector<int>& key)
{
    QVector<MorseElement> elements;
    for (int i = 0; i < ciphertext.size(); ++i) {
        int digit = ciphertext[i].digitValue();
        if (digit >= 0 && digit < key.size())
            elements.append(static_cast<MorseElement>(key[digit]));
    }
    return morseToText(elements);
}

/* ---- Prune key space using frequency constraints ---- */

QVector<QVector<int>> PolluxCode7::pruneKeySpace(const QString& ciphertext) const
{
    // Count digit frequencies
    QVector<int> freq(10, 0);
    for (int i = 0; i < ciphertext.size(); ++i) {
        int d = ciphertext[i].digitValue();
        if (d >= 0) freq[d]++;
    }

    // Constraint: dot should be most frequent, followed by dash, then separators
    // At least 2 digits must map to dot (most common element)
    QVector<QVector<int>> candidates;
    for (int dotMask = 0; dotMask < 1024; ++dotMask) {
        int dotCount = 0;
        for (int b = 0; b < 10; ++b)
            if (dotMask & (1 << b)) dotCount++;
        if (dotCount < 1 || dotCount > 4) continue;

        QVector<int> key(10, -1);
        for (int b = 0; b < 10; ++b)
            if (dotMask & (1 << b)) key[b] = Dot;

        // Check dot frequency constraint: top 2-3 digits by frequency should be dots
        int dotFreq = 0;
        for (int b = 0; b < 10; ++b)
            if (key[b] == Dot) dotFreq += freq[b];
        if (dotFreq < ciphertext.size() / 3) continue;

        // Assign remaining: at least one dash, one letterSep
        bool hasDash = false, hasSep = false;
        for (int dashDigit = 0; dashDigit < 10; ++dashDigit) {
            if (key[dashDigit] != -1) continue;
            key[dashDigit] = Dash;
            hasDash = true;
            for (int sepDigit = 0; sepDigit < 10; ++sepDigit) {
                if (key[sepDigit] != -1) continue;
                key[sepDigit] = LetterSep;
                hasSep = true;
                // Fill remaining with dot/dash
                for (int r = 0; r < 10; ++r)
                    if (key[r] == -1) key[r] = Dot;
                candidates.append(key);
                // Reset
                for (int r = 0; r < 10; ++r)
                    if (r != dashDigit && r != sepDigit && freq[r] == 0) key[r] = -1;
                hasSep = false;
            }
            hasDash = false;
            key[dashDigit] = -1;
        }
    }

    // Keep top candidates (limit search space)
    if (candidates.size() > 100)
        candidates = candidates.mid(0, 100);
    return candidates;
}

/* ---- Score plaintext by English frequency ---- */

double PolluxCode7::scorePlaintext(const QString& candidate) const
{
    // English letter frequencies (approximate)
    static const QMap<QChar, double> engFreq = {
        {'E',12.7},{'T',9.1},{'A',8.2},{'O',7.5},{'I',7.0},
        {'N',6.7},{'S',6.3},{'H',6.1},{'R',6.0},{'D',4.3},
        {'L',4.0},{'C',2.8},{'U',2.8},{'M',2.4},{'W',2.4},
        {'F',2.2},{'G',2.0},{'Y',2.0},{'P',1.9},{'B',1.5}
    };
    double score = 0.0;
    int alphaCount = 0;
    for (const QChar& ch : candidate) {
        QChar upper = ch.toUpper();
        if (engFreq.contains(upper)) {
            score += engFreq[upper];
            alphaCount++;
        }
    }
    return (alphaCount > 0) ? score / alphaCount : 0.0;
}

/* ---- DFS with pruning for key search ---- */

bool PolluxCode7::dfsSearch(const QVector<int>& cipher, int pos, QVector<int>& key,
                             QVector<MorseElement>& decoded, QString& bestText,
                             double& bestScore, int& explored)
{
    if (explored >= m_maxSearchDepth) return false;
    explored++;

    if (pos >= cipher.size()) {
        // Complete decode: score it
        QString text = morseToText(decoded);
        double score = scorePlaintext(text);
        if (score > bestScore) {
            bestScore = score;
            bestText = text;
        }
        return true;
    }

    int digit = cipher[pos];
    if (digit >= 0 && digit < key.size() && key[digit] != -1) {
        decoded.append(static_cast<MorseElement>(key[digit]));
        dfsSearch(cipher, pos + 1, key, decoded, bestText, bestScore, explored);
        decoded.removeLast();
    }
    return false;
}

/* ---- Auto-decrypt with constraint pruning ---- */

PolluxCode7::DecryptResult PolluxCode7::autoDecrypt(const QString& ciphertext)
{
    QElapsedTimer timer;
    timer.start();

    DecryptResult result;

    // Convert ciphertext digits
    QVector<int> cipherDigits;
    for (int i = 0; i < ciphertext.size(); ++i) {
        int d = ciphertext[i].digitValue();
        if (d >= 0) cipherDigits.append(d);
    }

    // Get pruned key candidates
    QVector<QVector<int>> candidates = pruneKeySpace(ciphertext);

    QString bestText;
    double bestScore = 0.0;
    int totalExplored = 0;

    for (const auto& key : candidates) {
        QVector<MorseElement> decoded;
        dfsSearch(cipherDigits, 0, const_cast<QVector<int>&>(key),
                  decoded, bestText, bestScore, totalExplored);
    }

    result.plaintext = bestText;
    result.confidence = qBound(0.0, bestScore / 8.0, 1.0);
    result.keysExplored = totalExplored;

    double elapsed = timer.elapsed();
    m_stats.messageLength = ciphertext.size();
    m_stats.keysExplored = totalExplored;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit decryptionDone(result.plaintext, result.confidence, elapsed);

    return result;
}

/* ---- Accessors ---- */

void PolluxCode7::setMaxSearchDepth(int depth) { m_maxSearchDepth = qBound(100, depth, 1000000); }

QMap<QChar, QString> PolluxCode7::morseTable() const { return m_morseEncode; }

/* ---- Reset ---- */

void PolluxCode7::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
