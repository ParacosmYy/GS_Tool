#include "b25981/m25981.h"
QVector<double> m25981::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
