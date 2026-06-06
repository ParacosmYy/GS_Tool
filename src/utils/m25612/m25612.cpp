#include "m25612/m25612.h"
QVector<double> m25612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
