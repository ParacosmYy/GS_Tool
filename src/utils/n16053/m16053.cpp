#include "n16053/m16053.h"
QVector<double> m16053::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
