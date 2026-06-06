#include "f21645/m21645.h"
QVector<double> m21645::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
