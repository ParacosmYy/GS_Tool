#include "o10854/m10854.h"
QVector<double> m10854::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
