#include "m24612/m24612.h"
QVector<double> m24612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
