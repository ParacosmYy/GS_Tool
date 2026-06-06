#include "m31212/m31212.h"
QVector<double> m31212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
