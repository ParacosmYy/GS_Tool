#include "l8351/m8351.h"
QVector<double> m8351::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
