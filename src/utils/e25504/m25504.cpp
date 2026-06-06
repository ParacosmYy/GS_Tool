#include "e25504/m25504.h"
QVector<double> m25504::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
