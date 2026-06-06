#include "m23032/m23032.h"
QVector<double> m23032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
