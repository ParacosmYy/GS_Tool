#include "g8006/m8006.h"
QVector<double> m8006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
