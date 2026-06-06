#include "m16752/m16752.h"
QVector<double> m16752::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
