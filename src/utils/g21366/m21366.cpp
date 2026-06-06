#include "g21366/m21366.h"
QVector<double> m21366::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
