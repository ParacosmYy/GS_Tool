#include "l35411/m35411.h"
QVector<double> m35411::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
