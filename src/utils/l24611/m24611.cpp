#include "l24611/m24611.h"
QVector<double> m24611::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
