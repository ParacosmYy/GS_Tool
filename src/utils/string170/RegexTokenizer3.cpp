/**
 * @file RegexTokenizer3.cpp
 * @brief Regex-based tokenizer for protocol data parsing implementation
 */
#include "string170/RegexTokenizer3.h"
#include <QElapsedTimer>

QVector<double> RegexTokenizer3::compute(const QVector<double> &input)
{
    QElapsedTimer t;
    t.start();
    m_stats.calls++;

    if (input.isEmpty()) {
        m_stats.errors++;
        return {};
    }

    QVector<double> result = input;
    m_stats.itemsProcessed += static_cast<quint64>(input.size());
    emit computed(result);
    Q_UNUSED(t)
    return result;
}

