#include "l32811/m32811.h"
QVector<double> m32811::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
