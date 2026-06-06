#include "l16111/m16111.h"
QVector<double> m16111::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
