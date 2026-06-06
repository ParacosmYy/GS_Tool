#include "e8204/m8204.h"
QVector<double> m8204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
