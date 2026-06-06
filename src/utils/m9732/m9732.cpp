#include "m9732/m9732.h"
QVector<double> m9732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
