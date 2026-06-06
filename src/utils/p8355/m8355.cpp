#include "p8355/m8355.h"
QVector<double> m8355::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
