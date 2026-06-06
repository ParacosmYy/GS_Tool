#include "a24700/m24700.h"
QVector<double> m24700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
