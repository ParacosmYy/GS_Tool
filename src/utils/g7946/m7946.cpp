#include "g7946/m7946.h"
QVector<double> m7946::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
