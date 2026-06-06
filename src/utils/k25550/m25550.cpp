#include "k25550/m25550.h"
QVector<double> m25550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
