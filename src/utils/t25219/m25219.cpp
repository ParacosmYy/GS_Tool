#include "t25219/m25219.h"
QVector<double> m25219::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
