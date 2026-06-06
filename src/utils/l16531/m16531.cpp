#include "l16531/m16531.h"
QVector<double> m16531::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
