#include "g21926/m21926.h"
QVector<double> m21926::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
