#include "l25871/m25871.h"
QVector<double> m25871::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
