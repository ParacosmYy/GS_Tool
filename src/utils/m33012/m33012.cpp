#include "m33012/m33012.h"
QVector<double> m33012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
