#include "d16883/m16883.h"
QVector<double> m16883::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
