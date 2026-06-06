#include "l18411/m18411.h"
QVector<double> m18411::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
