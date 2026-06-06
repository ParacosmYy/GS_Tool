#include "l16051/m16051.h"
QVector<double> m16051::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
