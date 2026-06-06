#include "m27512/m27512.h"
QVector<double> m27512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
