#include "k10550/m10550.h"
QVector<double> m10550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
