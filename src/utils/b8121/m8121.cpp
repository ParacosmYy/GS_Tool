#include "b8121/m8121.h"
QVector<double> m8121::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
