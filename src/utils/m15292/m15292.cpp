#include "m15292/m15292.h"
QVector<double> m15292::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
