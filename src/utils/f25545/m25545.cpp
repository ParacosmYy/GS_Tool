#include "f25545/m25545.h"
QVector<double> m25545::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
