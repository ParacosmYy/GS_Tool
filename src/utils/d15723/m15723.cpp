#include "d15723/m15723.h"
QVector<double> m15723::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
