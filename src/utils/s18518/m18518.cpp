#include "s18518/m18518.h"
QVector<double> m18518::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
