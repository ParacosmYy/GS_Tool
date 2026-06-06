#include "m26212/m26212.h"
QVector<double> m26212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
