#include "k36230/m36230.h"
QVector<double> m36230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
