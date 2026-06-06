#include "k36810/m36810.h"
QVector<double> m36810::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
