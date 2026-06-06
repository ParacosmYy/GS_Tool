#include "k24130/m24130.h"
QVector<double> m24130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
