#include "f15485/m15485.h"
QVector<double> m15485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
