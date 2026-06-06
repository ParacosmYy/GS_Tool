#include "f25525/m25525.h"
QVector<double> m25525::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
