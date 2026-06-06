#include "t25019/m25019.h"
QVector<double> m25019::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
