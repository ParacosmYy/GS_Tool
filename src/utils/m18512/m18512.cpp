#include "m18512/m18512.h"
QVector<double> m18512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
