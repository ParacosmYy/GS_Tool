#include "g25126/m25126.h"
QVector<double> m25126::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
