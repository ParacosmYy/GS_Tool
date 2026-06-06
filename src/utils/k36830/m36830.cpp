#include "k36830/m36830.h"
QVector<double> m36830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
