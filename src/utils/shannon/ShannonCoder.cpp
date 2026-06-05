/**
 * @file ShannonCoder.cpp
 * @brief Shannon-Fano编码器实现
 */

#include "utils/shannon/ShannonCoder.h"

#include <QElapsedTimer>
#include <QDataStream>
#include <QIODevice>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
ShannonCoder::ShannonCoder(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

/** @brief 编码 @param data 输入数据 @return 编码结果 */
QByteArray ShannonCoder::encode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) {
        m_stats.totalEncodes++;
        emit encoded({}, 0.0);
        return {};
    }

    QVector<quint32> freq = buildFreqTable(data);
    int total = data.size();
    buildCodeTable(freq, total);

    /* 编码到位流 */
    QByteArray bitStream;
    bitStream.reserve(data.size() * 4);
    for (int i = 0; i < data.size(); ++i) {
        int sym = static_cast<quint8>(data[i]);
        bitStream.append(m_codeTable.value(sym));
    }

    /* 位流打包 */
    QByteArray packed;
    int fullBytes = bitStream.size() / 8;
    for (int i = 0; i < fullBytes; ++i) {
        quint8 byte = 0;
        for (int b = 0; b < 8; ++b) {
            if (bitStream[i * 8 + b] == '1') byte |= (1 << (7 - b));
        }
        packed.append(static_cast<char>(byte));
    }
    int remain = bitStream.size() % 8;
    if (remain > 0) {
        quint8 byte = 0;
        for (int b = 0; b < remain; ++b) {
            if (bitStream[fullBytes * 8 + b] == '1') byte |= (1 << (7 - b));
        }
        packed.append(static_cast<char>(byte));
    }

    /* 序列化头部 */
    QByteArray header;
    QDataStream hs(&header, QIODevice::WriteOnly);
    hs << static_cast<quint32>(data.size());
    hs << static_cast<quint32>(bitStream.size());

    quint16 symbolCount = 0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) ++symbolCount;
    }
    hs << symbolCount;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            hs << static_cast<quint8>(i) << freq[i];
        }
    }

    QByteArray result = header + packed;

    /* 计算效率 */
    double H = 0.0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            double p = static_cast<double>(freq[i]) / total;
            H -= p * qLn(p) / qLn(2.0);
        }
    }
    double avgLen = (total > 0) ? static_cast<double>(bitStream.size()) / total : 0.0;
    double efficiency = (avgLen > 0) ? H / avgLen : 0.0;

    m_stats.totalEncodes++;
    const auto n = m_stats.totalEncodes;
    m_stats.avgEfficiency = (n == 1) ? efficiency :
        m_stats.avgEfficiency * (n - 1) / n + efficiency / n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / n;

    emit encoded(result, efficiency);
    return result;
}

/** @brief 解码 @param data 编码数据 @return 原始数据 */
QByteArray ShannonCoder::decode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.size() < 8) {
        emit error(tr("Shannon解码错误: 数据太短"));
        return {};
    }

    QDataStream hs(data);
    quint32 originalSize = 0;
    quint32 totalBits = 0;
    quint16 symbolCount = 0;
    hs >> originalSize >> totalBits >> symbolCount;

    QVector<quint32> freq(256, 0);
    for (quint16 i = 0; i < symbolCount; ++i) {
        quint8 sym = 0;
        quint32 f = 0;
        hs >> sym >> f;
        freq[sym] = f;
    }

    int headerSize = 8 + static_cast<int>(symbolCount) * 5 + 2;
    buildCodeTable(freq, originalSize);

    QMap<QByteArray, int> decodeMap;
    for (auto it = m_codeTable.constBegin();
         it != m_codeTable.constEnd(); ++it) {
        decodeMap[it.value()] = it.key();
    }

    QByteArray packed = data.mid(headerSize);
    QByteArray bitStream;
    bitStream.reserve(totalBits);
    for (int i = 0; i < packed.size() && bitStream.size() < static_cast<int>(totalBits); ++i) {
        quint8 byte = static_cast<quint8>(packed[i]);
        for (int b = 7; b >= 0 && bitStream.size() < static_cast<int>(totalBits); --b) {
            bitStream.append((byte & (1 << b)) ? '1' : '0');
        }
    }

    QByteArray result;
    result.reserve(originalSize);
    QByteArray currentCode;
    for (int i = 0; i < bitStream.size() && result.size() < static_cast<int>(originalSize); ++i) {
        currentCode.append(bitStream[i]);
        if (decodeMap.contains(currentCode)) {
            result.append(static_cast<char>(decodeMap[currentCode]));
            currentCode.clear();
        }
    }

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(m_stats.totalEncodes + m_stats.totalDecodes, 1ULL);

    emit decoded(result);
    return result;
}

/** @brief 计算Shannon熵 @param data 数据 @return 熵 */
double ShannonCoder::entropy(const QByteArray& data) const
{
    if (data.isEmpty()) return 0.0;
    QVector<quint32> freq = buildFreqTable(data);
    double H = 0.0;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) {
            double p = static_cast<double>(freq[i]) / data.size();
            H -= p * qLn(p) / qLn(2.0);
        }
    }
    return H;
}

/** @brief 获取符号表 @return 符号信息 */
QVector<ShannonCoder::SymbolInfo> ShannonCoder::symbolTable() const
{
    return m_symbolTable;
}

/** @brief 构建编码表 @param freq 频率 @param total 总数 */
void ShannonCoder::buildCodeTable(const QVector<quint32>& freq, int total)
{
    struct SymbolFreq { int symbol; quint32 freq; };
    QVector<SymbolFreq> symbols;
    for (int i = 0; i < 256; ++i) {
        if (freq[i] > 0) symbols.append({i, freq[i]});
    }
    std::sort(symbols.begin(), symbols.end(),
              [](const SymbolFreq& a, const SymbolFreq& b) {
                  return a.freq > b.freq;
              });

    if (symbols.isEmpty()) return;

    m_codeTable.clear();
    m_symbolTable.clear();

    std::function<void(int, int, QByteArray)> assign =
        [&](int left, int right, QByteArray code) {
            if (left + 1 == right) {
                m_codeTable[symbols[left].symbol] =
                    code.isEmpty() ? QByteArray(1, '0') : code;
                SymbolInfo info;
                info.symbol = symbols[left].symbol;
                info.probability = static_cast<double>(symbols[left].freq) / total;
                info.code = m_codeTable[symbols[left].symbol];
                m_symbolTable.append(info);
                return;
            }

            quint64 leftSum = 0, totalSum = 0;
            for (int i = left; i < right; ++i) totalSum += symbols[i].freq;

            int split = left;
            for (int i = left; i < right - 1; ++i) {
                leftSum += symbols[i].freq;
                if (leftSum >= totalSum / 2) {
                    split = i + 1;
                    break;
                }
            }
            if (split == left) split = left + 1;

            assign(left, split, code + '0');
            assign(split, right, code + '1');
        };

    assign(0, symbols.size(), QByteArray());
}

/** @brief 构建频率表 @param data 数据 @return 频率表 */
QVector<quint32> ShannonCoder::buildFreqTable(const QByteArray& data) const
{
    QVector<quint32> freq(256, 0);
    for (int i = 0; i < data.size(); ++i) {
        freq[static_cast<quint8>(data[i])]++;
    }
    return freq;
}

/** @brief 重置统计 */
void ShannonCoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
