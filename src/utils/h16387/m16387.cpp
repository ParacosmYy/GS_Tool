#include "h16387/m16387.h"
QVector<double> m16387::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
