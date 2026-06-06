#include "e14204/m14204.h"
QVector<double> m14204::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
