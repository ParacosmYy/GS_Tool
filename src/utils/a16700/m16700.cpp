#include "a16700/m16700.h"
QVector<double> m16700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
