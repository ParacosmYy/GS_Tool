#include "m30192/m30192.h"
QVector<double> m30192::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
