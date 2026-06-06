#include "g16126/m16126.h"
QVector<double> m16126::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
