#include "q16256/m16256.h"
QVector<double> m16256::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
