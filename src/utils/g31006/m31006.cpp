#include "g31006/m31006.h"
QVector<double> m31006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
