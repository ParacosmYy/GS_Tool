#include "n18253/m18253.h"
QVector<double> m18253::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
