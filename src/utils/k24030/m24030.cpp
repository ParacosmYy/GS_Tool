#include "k24030/m24030.h"
QVector<double> m24030::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
