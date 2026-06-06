#include "l9411/m9411.h"
QVector<double> m9411::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
