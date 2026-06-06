#include "m26852/m26852.h"
QVector<double> m26852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
