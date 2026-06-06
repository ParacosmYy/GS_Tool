#include "k36110/m36110.h"
QVector<double> m36110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
