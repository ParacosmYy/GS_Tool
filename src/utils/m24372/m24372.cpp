#include "m24372/m24372.h"
QVector<double> m24372::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
