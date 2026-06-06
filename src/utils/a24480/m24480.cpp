#include "a24480/m24480.h"
QVector<double> m24480::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
