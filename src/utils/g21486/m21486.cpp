#include "g21486/m21486.h"
QVector<double> m21486::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
