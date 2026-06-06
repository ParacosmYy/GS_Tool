#include "l20711/m20711.h"
QVector<double> m20711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
