#include "m15552/m15552.h"
QVector<double> m15552::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
