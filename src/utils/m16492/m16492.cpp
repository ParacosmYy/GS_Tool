#include "m16492/m16492.h"
QVector<double> m16492::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
