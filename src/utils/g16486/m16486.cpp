#include "g16486/m16486.h"
QVector<double> m16486::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
