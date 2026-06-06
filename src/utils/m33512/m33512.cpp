#include "m33512/m33512.h"
QVector<double> m33512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
