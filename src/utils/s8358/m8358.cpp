#include "s8358/m8358.h"
QVector<double> m8358::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
