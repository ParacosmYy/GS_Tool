#include "m18732/m18732.h"
QVector<double> m18732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
