#include "m32612/m32612.h"
QVector<double> m32612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
