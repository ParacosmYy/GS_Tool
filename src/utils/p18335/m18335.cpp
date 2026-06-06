#include "p18335/m18335.h"
QVector<double> m18335::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
