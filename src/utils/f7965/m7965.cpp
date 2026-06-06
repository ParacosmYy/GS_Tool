#include "f7965/m7965.h"
QVector<double> m7965::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
