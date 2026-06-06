#include "c8102/m8102.h"
QVector<double> m8102::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
