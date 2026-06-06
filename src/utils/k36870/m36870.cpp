#include "k36870/m36870.h"
QVector<double> m36870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
