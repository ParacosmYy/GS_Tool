#include "p25475/m25475.h"
QVector<double> m25475::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
