#include "l8411/m8411.h"
QVector<double> m8411::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
