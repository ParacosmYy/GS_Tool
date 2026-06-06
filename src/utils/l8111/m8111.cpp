#include "l8111/m8111.h"
QVector<double> m8111::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
