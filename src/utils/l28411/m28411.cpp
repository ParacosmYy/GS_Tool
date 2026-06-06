#include "l28411/m28411.h"
QVector<double> m28411::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
