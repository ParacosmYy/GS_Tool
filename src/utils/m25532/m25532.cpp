#include "m25532/m25532.h"
QVector<double> m25532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
