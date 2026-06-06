#include "l8871/m8871.h"
QVector<double> m8871::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
