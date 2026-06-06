#include "d9843/m9843.h"
QVector<double> m9843::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
