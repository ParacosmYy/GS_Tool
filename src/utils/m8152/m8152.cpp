#include "m8152/m8152.h"
QVector<double> m8152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
