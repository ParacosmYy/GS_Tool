#include "m31012/m31012.h"
QVector<double> m31012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
