#include "a16220/m16220.h"
QVector<double> m16220::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
