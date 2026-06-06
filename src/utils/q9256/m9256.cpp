#include "q9256/m9256.h"
QVector<double> m9256::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
