#include "k24350/m24350.h"
QVector<double> m24350::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
