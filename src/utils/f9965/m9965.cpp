#include "f9965/m9965.h"
QVector<double> m9965::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
