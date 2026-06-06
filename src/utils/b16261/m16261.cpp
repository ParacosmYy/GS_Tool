#include "b16261/m16261.h"
QVector<double> m16261::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
