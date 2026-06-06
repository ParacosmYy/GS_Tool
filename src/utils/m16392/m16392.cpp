#include "m16392/m16392.h"
QVector<double> m16392::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
