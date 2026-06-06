#include "l29411/m29411.h"
QVector<double> m29411::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
