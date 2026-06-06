#include "g21006/m21006.h"
QVector<double> m21006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
