#include "l8971/m8971.h"
QVector<double> m8971::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
