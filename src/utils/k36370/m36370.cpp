#include "k36370/m36370.h"
QVector<double> m36370::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
