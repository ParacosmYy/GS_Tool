#include "d10843/m10843.h"
QVector<double> m10843::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
