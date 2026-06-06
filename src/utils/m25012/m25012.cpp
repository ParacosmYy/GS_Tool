#include "m25012/m25012.h"
QVector<double> m25012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
