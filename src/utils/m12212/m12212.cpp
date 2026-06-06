#include "m12212/m12212.h"
QVector<double> m12212::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
