#include "e16604/m16604.h"
QVector<double> m16604::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
