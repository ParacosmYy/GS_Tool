#include "m21852/m21852.h"
QVector<double> m21852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
