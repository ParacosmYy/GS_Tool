#include "a33100/m33100.h"
QVector<double> m33100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
