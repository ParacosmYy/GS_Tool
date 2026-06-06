#include "d25723/m25723.h"
QVector<double> m25723::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
