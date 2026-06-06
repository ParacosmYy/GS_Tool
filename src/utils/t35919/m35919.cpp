#include "t35919/m35919.h"
QVector<double> m35919::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
