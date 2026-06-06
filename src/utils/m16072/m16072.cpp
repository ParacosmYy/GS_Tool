#include "m16072/m16072.h"
QVector<double> m16072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
