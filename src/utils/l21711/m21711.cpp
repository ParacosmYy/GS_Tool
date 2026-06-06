#include "l21711/m21711.h"
QVector<double> m21711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
