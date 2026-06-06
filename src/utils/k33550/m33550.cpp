#include "k33550/m33550.h"
QVector<double> m33550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
