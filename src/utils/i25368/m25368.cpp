#include "i25368/m25368.h"
QVector<double> m25368::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
