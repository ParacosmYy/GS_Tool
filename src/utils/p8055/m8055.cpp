#include "p8055/m8055.h"
QVector<double> m8055::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
