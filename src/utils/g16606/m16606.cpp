#include "g16606/m16606.h"
QVector<double> m16606::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
