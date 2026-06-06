#include "k7830/m7830.h"
QVector<double> m7830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
