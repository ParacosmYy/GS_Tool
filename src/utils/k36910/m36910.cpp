#include "k36910/m36910.h"
QVector<double> m36910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
