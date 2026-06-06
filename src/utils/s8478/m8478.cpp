#include "s8478/m8478.h"
QVector<double> m8478::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
