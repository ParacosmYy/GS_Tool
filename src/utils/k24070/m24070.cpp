#include "k24070/m24070.h"
QVector<double> m24070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
