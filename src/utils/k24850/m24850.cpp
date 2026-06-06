#include "k24850/m24850.h"
QVector<double> m24850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
