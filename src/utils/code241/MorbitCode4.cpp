/**
 * @file MorbitCode4.cpp
 * @brief MorbitCode4 实现
 *
 * 实现Morbit密码：三符号Morse片段编码与穷举密钥排列搜索解密。
 */

#include "utils/code241/MorbitCode4.h"

#include <QElapsedTimer>
#include <algorithm>

/* ---- Construction / Destruction ---- */

MorbitCode4::MorbitCode4(QObject *parent) : QObject(parent)
{
    buildMorseTable();
    buildFragmentTable();
    // Default key: identity permutation 1..9
    m_key = {1, 2, 3, 4, 5, 6, 7, 8, 9};
}

MorbitCode4::~MorbitCode4() = default;

/* ---- Build Morse table ---- */

void MorbitCode4::buildMorseTable()
{
    m_morseTable.clear();
    m_morseTable['A'] = ".-";    m_morseTable['B'] = "-...";
    m_morseTable['C'] = "-.-.";  m_morseTable['D'] = "-..";
    m_morseTable['E'] = ".";     m_morseTable['F'] = "..-.";
    m_morseTable['G'] = "--.";   m_morseTable['H'] = "....";
    m_morseTable['I'] = "..";    m_morseTable['J'] = ".---";
    m_morseTable['K'] = "-.-";   m_morseTable['L'] = ".-..";
    m_morseTable['M'] = "--";    m_morseTable['N'] = "-.";
    m_morseTable['O'] = "---";   m_morseTable['P'] = ".--.";
    m_morseTable['Q'] = "--.-";  m_morseTable['R'] = ".-.";
    m_morseTable['S'] = "...";   m_morseTable['T'] = "-";
    m_morseTable['U'] = "..-";   m_morseTable['V'] = "...-";
    m_morseTable['W'] = ".--";   m_morseTable['X'] = "-..-";
    m_morseTable['Y'] = "-.--";  m_morseTable['Z'] = "--..";
    m_morseTable['0'] = "-----"; m_morseTable['1'] = ".----";
    m_morseTable['2'] = "..---"; m_morseTable['3'] = "...--";
    m_morseTable['4'] = "....-"; m_morseTable['5'] = ".....";
    m_morseTable['6'] = "-...."; m_morseTable['7'] = "--...";
    m_morseTable['8'] = "---.."; m_morseTable['9'] = "----.";
}

/* ---- Build fragment table (9 Morse fragments) ---- */

void MorbitCode4::buildFragmentTable()
{
    // Standard Morbit 9-fragment encoding
    m_fragmentTable[1] = "..";
    m_fragmentTable[2] = ".-";
    m_fragmentTable[3] = ".x";  // x = char separator (space)
    m_fragmentTable[4] = "-.";
    m_fragmentTable[5] = "--";
    m_fragmentTable[6] = "-x";
    m_fragmentTable[7] = "x.";
    m_fragmentTable[8] = "x-";
    m_fragmentTable[9] = "xx";  // word separator
}

/* ---- Set key ---- */

void MorbitCode4::setKey(const QVector<int>& key)
{
    if (key.size() == 9) m_key = key;
}

/* ---- Text to Morse ---- */

QString MorbitCode4::textToMorse(const QString& text) const
{
    QString morse;
    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i].toUpper();
        if (m_morseTable.contains(ch)) {
            if (!morse.isEmpty()) morse += QLatin1Char(' ');
            morse += m_morseTable[ch];
        }
    }
    return morse;
}

/* ---- Morse to text ---- */

QString MorbitCode4::morseToText(const QString& morse) const
{
    // Build reverse lookup
    QMap<QString, QChar> reverse;
    for (auto it = m_morseTable.begin(); it != m_morseTable.end(); ++it)
        reverse[it.value()] = it.key();

    QString result;
    QStringList chars = morse.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    for (const QString& c : chars) {
        if (reverse.contains(c))
            result += reverse[c];
        else
            result += QLatin1Char('?');
    }
    return result;
}

/* ---- Encrypt ---- */

QString MorbitCode4::encrypt(const QString& plaintext) const
{
    QElapsedTimer timer;
    timer.start();

    QString morse = textToMorse(plaintext);
    // Pad morse to even length for 2-char fragment encoding
    if (morse.size() % 2 != 0) morse += QLatin1Char('x');

    QString cipher;
    // Build reverse fragment table: fragment -> digit
    QMap<QString, int> revFrag;
    for (auto it = m_fragmentTable.begin(); it != m_fragmentTable.end(); ++it)
        revFrag[it.value()] = it.key();

    for (int i = 0; i < morse.size(); i += 2) {
        QString frag = morse.mid(i, 2);
        if (revFrag.contains(frag)) {
            int digit = revFrag[frag];
            // Find position in key
            int pos = m_key.indexOf(digit);
            cipher += QString::number(pos + 1);
        }
    }

    const_cast<MorbitCode4*>(this)->m_stats.cipherLength = cipher.size();
    const_cast<MorbitCode4*>(this)->m_stats.keyLength = 9;
    const_cast<MorbitCode4*>(this)->m_stats.totalOps++;
    const_cast<MorbitCode4*>(this)->m_timeSum += timer.elapsed();
    const_cast<MorbitCode4*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    emit const_cast<MorbitCode4*>(this)->encryptionCompleted(plaintext.size(), cipher.size(), timer.elapsed());
    return cipher;
}

/* ---- Decrypt ---- */

QString MorbitCode4::decrypt(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QString morse;
    for (int i = 0; i < ciphertext.size(); ++i) {
        int pos = ciphertext[i].digitValue() - 1;
        if (pos >= 0 && pos < m_key.size()) {
            int digit = m_key[pos];
            if (m_fragmentTable.contains(digit))
                morse += m_fragmentTable[digit];
        }
    }

    QString result = morseToText(morse);
    return result;
}

/* ---- Score plaintext (English frequency analysis) ---- */

double MorbitCode4::scorePlaintext(const QString& text) const
{
    // English letter frequencies
    static const QMap<QChar, double> freq = {
        {'E', 12.70}, {'T', 9.06}, {'A', 8.17}, {'O', 7.51}, {'I', 6.97},
        {'N', 6.75}, {'S', 6.33}, {'H', 6.09}, {'R', 5.99}, {'D', 4.25},
        {'L', 4.03}, {'C', 2.78}, {'U', 2.76}, {'M', 2.41}, {'W', 2.36},
        {'F', 2.23}, {'G', 2.02}, {'Y', 1.97}, {'P', 1.93}, {'B', 1.29},
        {'V', 0.98}, {'K', 0.77}, {'J', 0.15}, {'X', 0.15}, {'Q', 0.10},
        {'Z', 0.07}
    };

    double score = 0.0;
    int alpha = 0;
    for (int i = 0; i < text.size(); ++i) {
        QChar ch = text[i].toUpper();
        if (freq.contains(ch)) {
            score += freq[ch];
            alpha++;
        } else if (ch == QLatin1Char(' ')) {
            score += 5.0;
        }
    }
    return (alpha > 0) ? score / text.size() : 0.0;
}

/* ---- Next permutation ---- */

bool MorbitCode4::nextPermutation(QVector<int>& arr) const
{
    int n = arr.size();
    int i = n - 2;
    while (i >= 0 && arr[i] >= arr[i + 1]) --i;
    if (i < 0) return false;
    int j = n - 1;
    while (arr[j] <= arr[i]) --j;
    std::swap(arr[i], arr[j]);
    int lo = i + 1, hi = n - 1;
    while (lo < hi) { std::swap(arr[lo], arr[hi]); ++lo; --hi; }
    return true;
}

/* ---- Factorial ---- */

qint64 MorbitCode4::factorial(int n)
{
    qint64 r = 1;
    for (int i = 2; i <= n; ++i) r *= i;
    return r;
}

/* ---- Brute force ---- */

QVector<QPair<QVector<int>, QString>> MorbitCode4::bruteForce(const QString& ciphertext) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<QVector<int>, QString>> results;
    QVector<int> perm = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    qint64 total = factorial(9);
    qint64 tested = 0;

    do {
        MorbitCode4 temp;
        temp.setKey(perm);
        temp.buildMorseTable();
        temp.buildFragmentTable();
        QString plain = temp.decrypt(ciphertext);
        double score = scorePlaintext(plain);

        if (score > 50.0)  // Threshold for promising results
            results.append({perm, plain});

        ++tested;
        if (tested % 100000 == 0)
            emit const_cast<MorbitCode4*>(this)->bruteForceProgress(static_cast<int>(tested), static_cast<int>(total));

    } while (nextPermutation(perm));

    // Sort by score descending
    std::sort(results.begin(), results.end(), [this](const auto& a, const auto& b) {
        return scorePlaintext(a.second) > scorePlaintext(b.second);
    });

    const_cast<MorbitCode4*>(this)->m_stats.permutationsTested = static_cast<int>(tested);
    const_cast<MorbitCode4*>(this)->m_stats.totalOps++;
    const_cast<MorbitCode4*>(this)->m_timeSum += timer.elapsed();
    const_cast<MorbitCode4*>(this)->m_stats.avgProcessingTimeMs =
        m_timeSum / m_stats.totalOps;

    return results;
}

/* ---- Reset ---- */

void MorbitCode4::resetStatistics()
{
    m_stats = Stats{}; m_timeSum = 0.0;
}
