/**
 * @file ArithmeticCode.cpp
 * @brief ArithmeticCode 实现
 *
 * 实现算术编解码：自适应频率模型、整数区间重归一化、位级I/O。
 */

#include "utils/code169/ArithmeticCode.h"

#include <QElapsedTimer>
#include <QtGlobal>

/* ---- FrequencyModel ---- */

void ArithmeticCode::FrequencyModel::reset(quint32 maxSym)
{
    maxSymbol = maxSym;
    freq.fill(1, maxSym + 2);  /* sentinel at end */
    total = maxSym + 2;
}

void ArithmeticCode::FrequencyModel::update(quint32 symbol)
{
    if (symbol > maxSymbol) return;
    freq[symbol]++;
    total++;
}

quint32 ArithmeticCode::FrequencyModel::cumulative(quint32 symbol) const
{
    quint32 sum = 0;
    for (quint32 i = 0; i < symbol; ++i)
        sum += freq[i];
    return sum;
}

/* ---- EncoderState / DecoderState ---- */

void ArithmeticCode::EncoderState::init()
{
    low = 0;
    high = kMask;
    pendingBits = 0;
    output.clear();
}

void ArithmeticCode::DecoderState::init(const QByteArray& input)
{
    low = 0;
    high = kMask;
    data = input.constData();
    dataLen = input.size();
    bytePos = 0;
    bitPos = 7;
    code = 0;
    /* Preload first 32 bits */
    for (int i = 0; i < 32; ++i)
        code = (code << 1) | readBit();
}

quint32 ArithmeticCode::DecoderState::readBit()
{
    if (bytePos >= dataLen) return 0;
    quint32 bit = (static_cast<quint8>(data[bytePos]) >> bitPos) & 1;
    if (bitPos == 0) { bytePos++; bitPos = 7; }
    else { bitPos--; }
    return bit;
}

/* ---- Construction ---- */

ArithmeticCode::ArithmeticCode(quint32 maxSymbol, QObject *parent)
    : QObject(parent), m_maxSymbol(maxSymbol)
{
}

ArithmeticCode::~ArithmeticCode() = default;

void ArithmeticCode::setMaxSymbol(quint32 maxSym) { m_maxSymbol = maxSym; }

/* ---- Bit output helper ---- */

void ArithmeticCode::outputBit(EncoderState& st, quint32 bit)
{
    /* Append bit to output byte buffer */
    if (st.output.isEmpty() || st.pendingBits == 0) {
        /* Triggered via pendingBits counting in renormalize */
    }
    Q_UNUSED(bit)
}

void ArithmeticCode::encodeRenormalize(EncoderState& st)
{
    while (true) {
        /* Both in upper half */
        if (st.high < kHalf) {
            st.output.append(char((st.low >> 31) & 1) + '0');
            for (quint32 i = 0; i < st.pendingBits; ++i)
                st.output.append(char(((st.low >> 31) & 1) ^ 1) + '0');
            st.pendingBits = 0;
        }
        /* Both in lower half */
        else if (st.low >= kHalf) {
            st.output.append('1');
            for (quint32 i = 0; i < st.pendingBits; ++i)
                st.output.append('0');
            st.pendingBits = 0;
        }
        /* E3: straddle */
        else if (st.low >= kQuarter && st.high < kThreeQuarter) {
            st.pendingBits++;
        }
        else {
            break;
        }

        st.low = st.low << 1;
        st.high = (st.high << 1) | 1;

        if (st.low >= kWhole) st.low -= kWhole;
        if (st.high >= kWhole) st.high -= kWhole;
    }
}

void ArithmeticCode::decodeRenormalize(DecoderState& st)
{
    while (true) {
        if (st.high < kHalf) {
            /* Both in lower half — scale up */
        } else if (st.low >= kHalf) {
            st.code -= kHalf;
            st.low -= kHalf;
            st.high -= kHalf;
        } else if (st.low >= kQuarter && st.high < kThreeQuarter) {
            st.code -= kQuarter;
            st.low -= kQuarter;
            st.high -= kQuarter;
        } else {
            break;
        }

        st.low = st.low << 1;
        st.high = (st.high << 1) | 1;
        st.code = (st.code << 1) | st.readBit();
    }
}

/* ---- Encode single symbol ---- */

void ArithmeticCode::encodeSymbol(EncoderState& st, quint32 symbol, FrequencyModel& model)
{
    quint32 range = st.high - st.low + 1;
    quint64 scaledLow = static_cast<quint64>(st.low)
        + (static_cast<quint64>(range) * model.cumulative(symbol) / model.total);
    quint64 scaledHigh = static_cast<quint64>(st.low)
        + (static_cast<quint64>(range) * model.cumulative(symbol + 1) / model.total) - 1;

    st.low = static_cast<quint32>(scaledLow & kMask);
    st.high = static_cast<quint32>(scaledHigh & kMask);

    model.update(symbol);
    encodeRenormalize(st);
}

/* ---- Decode single symbol ---- */

quint32 ArithmeticCode::decodeSymbol(DecoderState& st, FrequencyModel& model)
{
    quint32 range = st.high - st.low + 1;
    /* Find symbol by cumulative frequency */
    quint32 scaledValue = static_cast<quint32>(
        (static_cast<quint64>(st.code - st.low) * model.total / range));

    /* Linear search for symbol */
    quint32 symbol = 0;
    quint32 cumSum = 0;
    for (quint32 s = 0; s <= model.maxSymbol; ++s) {
        cumSum += model.freq[s];
        if (cumSum > scaledValue) { symbol = s; break; }
    }

    /* Narrow interval */
    quint64 newLow = static_cast<quint64>(st.low)
        + (static_cast<quint64>(range) * model.cumulative(symbol) / model.total);
    quint64 newHigh = static_cast<quint64>(st.low)
        + (static_cast<quint64>(range) * model.cumulative(symbol + 1) / model.total) - 1;

    st.low = static_cast<quint32>(newLow & kMask);
    st.high = static_cast<quint32>(newHigh & kMask);

    model.update(symbol);
    decodeRenormalize(st);
    return symbol;
}

/* ---- Flush encoder ---- */

void ArithmeticCode::flushEncoder(EncoderState& st)
{
    st.pendingBits++;
    if (st.low < kQuarter) {
        st.output.append('0');
        for (quint32 i = 0; i < st.pendingBits; ++i)
            st.output.append('1');
    } else {
        st.output.append('1');
        for (quint32 i = 0; i < st.pendingBits; ++i)
            st.output.append('0');
    }
}

/* ---- Public API ---- */

QByteArray ArithmeticCode::encode(const QVector<quint32>& symbols)
{
    QElapsedTimer timer;
    timer.start();

    FrequencyModel model;
    model.reset(m_maxSymbol);

    EncoderState st;
    st.init();

    for (quint32 sym : symbols)
        encodeSymbol(st, sym, model);

    flushEncoder(st);

    /* Convert bit string to bytes */
    QByteArray result;
    quint8 byte = 0;
    int bitCount = 0;
    for (char c : st.output) {
        byte = (byte << 1) | (c == '1' ? 1 : 0);
        bitCount++;
        if (bitCount == 8) { result.append(byte); byte = 0; bitCount = 0; }
    }
    if (bitCount > 0) {
        byte <<= (8 - bitCount);
        result.append(byte);
    }

    m_stats.totalEncodes++;
    m_stats.totalBitsEncoded += st.output.size();
    if (!symbols.isEmpty())
        m_stats.lastCompressionRatio = static_cast<double>(result.size())
            / symbols.size();
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit encodeCompleted(st.output.size());
    return result;
}

QVector<quint32> ArithmeticCode::decode(const QByteArray& data, int count)
{
    QElapsedTimer timer;
    timer.start();

    FrequencyModel model;
    model.reset(m_maxSymbol);

    DecoderState st;
    st.init(data);

    QVector<quint32> symbols;
    symbols.reserve(count);
    for (int i = 0; i < count; ++i)
        symbols.append(decodeSymbol(st, model));

    m_stats.totalDecodes++;
    m_timeSum += timer.elapsed();
    quint64 total = m_stats.totalEncodes + m_stats.totalDecodes;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit decodeCompleted(count);
    return symbols;
}

void ArithmeticCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
