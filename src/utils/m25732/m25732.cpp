#include "m25732/m25732.h"
QVector<double> m25732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
