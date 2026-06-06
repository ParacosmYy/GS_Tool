#include "e18284/m18284.h"
QVector<double> m18284::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
