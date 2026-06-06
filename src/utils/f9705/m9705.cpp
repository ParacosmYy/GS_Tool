#include "f9705/m9705.h"
QVector<double> m9705::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
