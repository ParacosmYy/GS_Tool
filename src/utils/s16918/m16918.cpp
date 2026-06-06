#include "s16918/m16918.h"
QVector<double> m16918::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
