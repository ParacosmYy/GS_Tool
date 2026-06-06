#include "t12859/m12859.h"
QVector<double> m12859::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
