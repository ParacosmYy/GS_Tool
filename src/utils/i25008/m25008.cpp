#include "i25008/m25008.h"
QVector<double> m25008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
