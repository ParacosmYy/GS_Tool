#include "c16602/m16602.h"
QVector<double> m16602::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
