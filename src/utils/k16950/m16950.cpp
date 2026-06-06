#include "k16950/m16950.h"
QVector<double> m16950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
