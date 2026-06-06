#include "l20511/m20511.h"
QVector<double> m20511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
