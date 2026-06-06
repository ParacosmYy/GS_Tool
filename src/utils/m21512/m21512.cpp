#include "m21512/m21512.h"
QVector<double> m21512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
