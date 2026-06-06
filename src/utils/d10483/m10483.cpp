#include "d10483/m10483.h"
QVector<double> m10483::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
