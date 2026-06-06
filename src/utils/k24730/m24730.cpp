#include "k24730/m24730.h"
QVector<double> m24730::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
