#include "m15732/m15732.h"
QVector<double> m15732::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
