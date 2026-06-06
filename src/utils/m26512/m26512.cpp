#include "m26512/m26512.h"
QVector<double> m26512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
