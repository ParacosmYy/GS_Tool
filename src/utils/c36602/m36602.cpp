#include "c36602/m36602.h"
QVector<double> m36602::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
