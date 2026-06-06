#include "g16726/m16726.h"
QVector<double> m16726::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
