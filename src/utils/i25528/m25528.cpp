#include "i25528/m25528.h"
QVector<double> m25528::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
