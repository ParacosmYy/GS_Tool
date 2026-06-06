#include "m31532/m31532.h"
QVector<double> m31532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
