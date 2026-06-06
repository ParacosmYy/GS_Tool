#include "m25032/m25032.h"
QVector<double> m25032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
