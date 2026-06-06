#include "f28545/m28545.h"
QVector<double> m28545::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
