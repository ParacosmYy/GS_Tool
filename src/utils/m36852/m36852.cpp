#include "m36852/m36852.h"
QVector<double> m36852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
