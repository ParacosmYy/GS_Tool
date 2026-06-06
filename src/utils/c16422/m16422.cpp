#include "c16422/m16422.h"
QVector<double> m16422::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
