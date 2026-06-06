#include "d18723/m18723.h"
QVector<double> m18723::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
