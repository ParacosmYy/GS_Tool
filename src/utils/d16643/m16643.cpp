#include "d16643/m16643.h"
QVector<double> m16643::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
