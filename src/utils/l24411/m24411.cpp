#include "l24411/m24411.h"
QVector<double> m24411::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
