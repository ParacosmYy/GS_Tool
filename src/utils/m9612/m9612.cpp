#include "m9612/m9612.h"
QVector<double> m9612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
