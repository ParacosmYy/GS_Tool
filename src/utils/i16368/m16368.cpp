#include "i16368/m16368.h"
QVector<double> m16368::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
