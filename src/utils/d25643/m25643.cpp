#include "d25643/m25643.h"
QVector<double> m25643::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
