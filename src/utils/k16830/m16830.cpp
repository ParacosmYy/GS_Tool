#include "k16830/m16830.h"
QVector<double> m16830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
