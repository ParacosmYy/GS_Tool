#include "m31272/m31272.h"
QVector<double> m31272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
