#include "i21808/m21808.h"
QVector<double> m21808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
