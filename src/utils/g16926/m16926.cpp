#include "g16926/m16926.h"
QVector<double> m16926::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
