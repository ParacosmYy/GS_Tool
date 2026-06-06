#include "m14512/m14512.h"
QVector<double> m14512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
