/**
 * @file AffinityProp2.cpp
 * @brief Affinity propagation clustering algorithm implementation
 */
#include "cluster161/AffinityProp2.h"
#include <QElapsedTimer>

QVector<double> AffinityProp2::compute(const QVector<double> &input)
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

