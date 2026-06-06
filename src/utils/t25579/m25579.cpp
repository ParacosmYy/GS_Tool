#include "t25579/m25579.h"
QVector<double> m25579::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
