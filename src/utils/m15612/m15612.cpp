#include "m15612/m15612.h"
QVector<double> m15612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
