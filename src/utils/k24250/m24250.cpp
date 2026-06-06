#include "k24250/m24250.h"
QVector<double> m24250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
