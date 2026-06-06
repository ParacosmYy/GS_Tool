#include "m23612/m23612.h"
QVector<double> m23612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
