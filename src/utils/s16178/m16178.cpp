#include "s16178/m16178.h"
QVector<double> m16178::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
