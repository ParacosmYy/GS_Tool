#include "l16411/m16411.h"
QVector<double> m16411::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
