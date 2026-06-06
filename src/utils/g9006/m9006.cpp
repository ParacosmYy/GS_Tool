#include "g9006/m9006.h"
QVector<double> m9006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
