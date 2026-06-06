#include "d16243/m16243.h"
QVector<double> m16243::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
