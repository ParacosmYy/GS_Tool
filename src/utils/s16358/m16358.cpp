#include "s16358/m16358.h"
QVector<double> m16358::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
