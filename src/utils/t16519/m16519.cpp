#include "t16519/m16519.h"
QVector<double> m16519::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
