#include "m26612/m26612.h"
QVector<double> m26612::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
