#include "f35545/m35545.h"
QVector<double> m35545::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
