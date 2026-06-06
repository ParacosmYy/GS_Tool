#include "m16092/m16092.h"
QVector<double> m16092::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
