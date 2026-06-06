#include "m16312/m16312.h"
QVector<double> m16312::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
