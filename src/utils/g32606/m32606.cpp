#include "g32606/m32606.h"
QVector<double> m32606::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
