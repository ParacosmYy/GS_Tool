#include "e25544/m25544.h"
QVector<double> m25544::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
