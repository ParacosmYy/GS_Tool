#include "o16834/m16834.h"
QVector<double> m16834::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
