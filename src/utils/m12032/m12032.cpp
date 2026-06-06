#include "m12032/m12032.h"
QVector<double> m12032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
