#include "f37505/m37505.h"
QVector<double> m37505::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
