#include "d16203/m16203.h"
QVector<double> m16203::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
