#include "d8603/m8603.h"
QVector<double> m8603::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
