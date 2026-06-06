#include "l36271/m36271.h"
QVector<double> m36271::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
