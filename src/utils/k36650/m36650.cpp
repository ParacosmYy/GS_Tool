#include "k36650/m36650.h"
QVector<double> m36650::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
