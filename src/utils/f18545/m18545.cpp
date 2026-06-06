#include "f18545/m18545.h"
QVector<double> m18545::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
