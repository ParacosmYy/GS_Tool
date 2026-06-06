#include "p8875/m8875.h"
QVector<double> m8875::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
