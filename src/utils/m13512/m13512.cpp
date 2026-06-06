#include "m13512/m13512.h"
QVector<double> m13512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
