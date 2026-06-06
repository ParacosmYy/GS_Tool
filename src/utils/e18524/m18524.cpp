#include "e18524/m18524.h"
QVector<double> m18524::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
