#include "t9219/m9219.h"
QVector<double> m9219::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
