#include "g18006/m18006.h"
QVector<double> m18006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
