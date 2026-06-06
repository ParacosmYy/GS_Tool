#include "d18123/m18123.h"
QVector<double> m18123::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
