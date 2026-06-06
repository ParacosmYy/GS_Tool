#include "m9512/m9512.h"
QVector<double> m9512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
