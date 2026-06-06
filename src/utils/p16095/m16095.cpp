#include "p16095/m16095.h"
QVector<double> m16095::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
