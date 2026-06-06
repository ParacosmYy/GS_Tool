#include "c16382/m16382.h"
QVector<double> m16382::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
