#include "a16480/m16480.h"
QVector<double> m16480::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
