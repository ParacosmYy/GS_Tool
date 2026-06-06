#include "m15152/m15152.h"
QVector<double> m15152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
