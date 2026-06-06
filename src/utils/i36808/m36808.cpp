#include "i36808/m36808.h"
QVector<double> m36808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
