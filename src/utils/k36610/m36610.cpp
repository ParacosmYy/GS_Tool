#include "k36610/m36610.h"
QVector<double> m36610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
