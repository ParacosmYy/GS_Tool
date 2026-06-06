#include "p16335/m16335.h"
QVector<double> m16335::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
