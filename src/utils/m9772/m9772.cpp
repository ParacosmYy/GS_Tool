#include "m9772/m9772.h"
QVector<double> m9772::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
