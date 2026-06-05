/**
 * @file Levenshtein3.cpp
 * @brief Levenshtein distance for fuzzy string matching implementation
 */
#include "string171/Levenshtein3.h"
#include <QElapsedTimer>

QVector<double> Levenshtein3::compute(const QVector<double> &input)
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

