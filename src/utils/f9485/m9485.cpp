#include "f9485/m9485.h"
QVector<double> m9485::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
