#include "e16064/m16064.h"
QVector<double> m16064::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
