#include "f24545/m24545.h"
QVector<double> m24545::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
