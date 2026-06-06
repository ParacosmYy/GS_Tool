#include "h25807/m25807.h"
QVector<double> m25807::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
