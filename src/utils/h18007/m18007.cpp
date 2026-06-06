#include "h18007/m18007.h"
QVector<double> m18007::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
