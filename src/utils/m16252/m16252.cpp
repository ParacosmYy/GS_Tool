#include "m16252/m16252.h"
QVector<double> m16252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
