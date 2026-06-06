#include "k16170/m16170.h"
QVector<double> m16170::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
