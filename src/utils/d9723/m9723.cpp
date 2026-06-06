#include "d9723/m9723.h"
QVector<double> m9723::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
