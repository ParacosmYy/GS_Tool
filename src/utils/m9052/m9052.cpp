#include "m9052/m9052.h"
QVector<double> m9052::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
