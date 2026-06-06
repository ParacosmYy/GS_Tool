#include "h25527/m25527.h"
QVector<double> m25527::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
