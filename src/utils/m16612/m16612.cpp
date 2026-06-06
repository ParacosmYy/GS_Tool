#include "m16612/m16612.h"
QVector<double> m16612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
