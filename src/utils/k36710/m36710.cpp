#include "k36710/m36710.h"
QVector<double> m36710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
