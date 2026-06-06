#include "a18480/m18480.h"
QVector<double> m18480::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
