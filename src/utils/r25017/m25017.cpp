#include "r25017/m25017.h"
QVector<double> m25017::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
