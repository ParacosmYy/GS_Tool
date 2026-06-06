#include "g21266/m21266.h"
QVector<double> m21266::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
