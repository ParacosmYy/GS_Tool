#include "m15852/m15852.h"
QVector<double> m15852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
