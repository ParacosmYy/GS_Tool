#include "d18603/m18603.h"
QVector<double> m18603::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
