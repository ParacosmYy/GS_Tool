#include "k16930/m16930.h"
QVector<double> m16930::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
