#include "m14612/m14612.h"
QVector<double> m14612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
