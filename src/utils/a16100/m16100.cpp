#include "a16100/m16100.h"
QVector<double> m16100::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
