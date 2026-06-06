#include "m23532/m23532.h"
QVector<double> m23532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
