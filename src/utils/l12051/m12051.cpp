#include "l12051/m12051.h"
QVector<double> m12051::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
