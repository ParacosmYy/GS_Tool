#include "k10730/m10730.h"
QVector<double> m10730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
