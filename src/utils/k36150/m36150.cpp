#include "k36150/m36150.h"
QVector<double> m36150::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
