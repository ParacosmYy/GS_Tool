#include "k24390/m24390.h"
QVector<double> m24390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
