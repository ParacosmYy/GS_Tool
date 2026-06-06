#include "m12812/m12812.h"
QVector<double> m12812::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
