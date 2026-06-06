#include "a16400/m16400.h"
QVector<double> m16400::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
