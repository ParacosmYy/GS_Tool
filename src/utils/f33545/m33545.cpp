#include "f33545/m33545.h"
QVector<double> m33545::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
