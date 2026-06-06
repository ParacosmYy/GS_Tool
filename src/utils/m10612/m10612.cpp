#include "m10612/m10612.h"
QVector<double> m10612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
