#include "m25972/m25972.h"
QVector<double> m25972::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
