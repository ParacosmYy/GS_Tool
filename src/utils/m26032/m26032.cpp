#include "m26032/m26032.h"
QVector<double> m26032::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
