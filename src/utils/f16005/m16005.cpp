#include "f16005/m16005.h"
QVector<double> m16005::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
