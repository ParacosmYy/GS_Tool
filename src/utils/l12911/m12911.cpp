#include "l12911/m12911.h"
QVector<double> m12911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
