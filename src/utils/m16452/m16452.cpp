#include "m16452/m16452.h"
QVector<double> m16452::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
