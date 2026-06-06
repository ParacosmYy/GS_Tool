#include "k12630/m12630.h"
QVector<double> m12630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
