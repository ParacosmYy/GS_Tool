#include "m9032/m9032.h"
QVector<double> m9032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
