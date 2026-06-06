#include "l36411/m36411.h"
QVector<double> m36411::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
