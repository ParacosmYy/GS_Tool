#include "e16224/m16224.h"
QVector<double> m16224::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
