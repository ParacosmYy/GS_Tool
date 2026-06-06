#include "m17732/m17732.h"
QVector<double> m17732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
