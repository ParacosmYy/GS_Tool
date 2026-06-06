#include "m22032/m22032.h"
QVector<double> m22032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
