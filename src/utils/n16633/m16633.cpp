#include "n16633/m16633.h"
QVector<double> m16633::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
