#include "k36250/m36250.h"
QVector<double> m36250::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
