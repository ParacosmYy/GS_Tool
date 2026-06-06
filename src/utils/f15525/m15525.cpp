#include "f15525/m15525.h"
QVector<double> m15525::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
