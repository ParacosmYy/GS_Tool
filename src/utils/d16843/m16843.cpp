#include "d16843/m16843.h"
QVector<double> m16843::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
