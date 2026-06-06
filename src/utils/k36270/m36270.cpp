#include "k36270/m36270.h"
QVector<double> m36270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
