#include "d18843/m18843.h"
QVector<double> m18843::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
