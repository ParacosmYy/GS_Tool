#include "l32111/m32111.h"
QVector<double> m32111::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
