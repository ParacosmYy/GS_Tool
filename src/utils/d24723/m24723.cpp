#include "d24723/m24723.h"
QVector<double> m24723::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
