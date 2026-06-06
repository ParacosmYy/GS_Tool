#include "k24230/m24230.h"
QVector<double> m24230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
