#include "m33032/m33032.h"
QVector<double> m33032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
