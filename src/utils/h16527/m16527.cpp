#include "h16527/m16527.h"
QVector<double> m16527::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
