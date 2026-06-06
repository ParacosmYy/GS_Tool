#include "k36550/m36550.h"
QVector<double> m36550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
