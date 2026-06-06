#include "f33485/m33485.h"
QVector<double> m33485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
