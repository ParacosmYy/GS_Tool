#include "g9946/m9946.h"
QVector<double> m9946::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
