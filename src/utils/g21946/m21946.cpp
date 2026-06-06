#include "g21946/m21946.h"
QVector<double> m21946::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
