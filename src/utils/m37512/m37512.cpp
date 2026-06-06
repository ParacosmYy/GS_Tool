#include "m37512/m37512.h"
QVector<double> m37512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
