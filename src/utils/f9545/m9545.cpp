#include "f9545/m9545.h"
QVector<double> m9545::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
