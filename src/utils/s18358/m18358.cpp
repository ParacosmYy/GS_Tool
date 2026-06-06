#include "s18358/m18358.h"
QVector<double> m18358::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
