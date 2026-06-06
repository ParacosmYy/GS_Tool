#include "l15411/m15411.h"
QVector<double> m15411::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
