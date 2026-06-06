#include "g35006/m35006.h"
QVector<double> m35006::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
