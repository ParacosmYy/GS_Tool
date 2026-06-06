#include "m16212/m16212.h"
QVector<double> m16212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
