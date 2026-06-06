#include "m23012/m23012.h"
QVector<double> m23012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
