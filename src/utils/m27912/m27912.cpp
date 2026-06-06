#include "m27912/m27912.h"
QVector<double> m27912::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
