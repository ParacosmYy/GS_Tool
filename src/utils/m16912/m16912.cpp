#include "m16912/m16912.h"
QVector<double> m16912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
