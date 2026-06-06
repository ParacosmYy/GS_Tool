#include "m13032/m13032.h"
QVector<double> m13032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
