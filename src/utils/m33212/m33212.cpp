#include "m33212/m33212.h"
QVector<double> m33212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
