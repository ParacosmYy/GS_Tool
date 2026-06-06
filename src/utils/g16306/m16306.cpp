#include "g16306/m16306.h"
QVector<double> m16306::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
