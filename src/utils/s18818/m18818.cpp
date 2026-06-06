#include "s18818/m18818.h"
QVector<double> m18818::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
