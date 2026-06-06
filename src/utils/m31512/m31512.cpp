#include "m31512/m31512.h"
QVector<double> m31512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
