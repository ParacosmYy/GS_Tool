#include "i12328/m12328.h"
QVector<double> m12328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
