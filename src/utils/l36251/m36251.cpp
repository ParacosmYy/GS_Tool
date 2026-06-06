#include "l36251/m36251.h"
QVector<double> m36251::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
