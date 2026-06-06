#include "m27612/m27612.h"
QVector<double> m27612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
