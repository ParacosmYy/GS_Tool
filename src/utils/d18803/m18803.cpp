#include "d18803/m18803.h"
QVector<double> m18803::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
