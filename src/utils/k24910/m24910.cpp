#include "k24910/m24910.h"
QVector<double> m24910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
