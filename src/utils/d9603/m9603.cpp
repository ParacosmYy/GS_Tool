#include "d9603/m9603.h"
QVector<double> m9603::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
