#include "m20612/m20612.h"
QVector<double> m20612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
