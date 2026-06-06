#include "m36732/m36732.h"
QVector<double> m36732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
