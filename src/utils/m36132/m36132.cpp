#include "m36132/m36132.h"
QVector<double> m36132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
