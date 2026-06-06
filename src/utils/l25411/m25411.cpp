#include "l25411/m25411.h"
QVector<double> m25411::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
