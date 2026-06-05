/**
 * @file BilinearInterp3.cpp
 * @brief Bilinear interpolation for 2D lookup tables implementation
 */
#include "interp171/BilinearInterp3.h"
#include <QElapsedTimer>

QVector<double> BilinearInterp3::compute(const QVector<double> &input)
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

