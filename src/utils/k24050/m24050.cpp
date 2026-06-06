#include "k24050/m24050.h"
QVector<double> m24050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
