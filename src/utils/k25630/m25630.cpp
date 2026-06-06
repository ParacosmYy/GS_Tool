#include "k25630/m25630.h"
QVector<double> m25630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
