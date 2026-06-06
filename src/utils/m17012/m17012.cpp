#include "m17012/m17012.h"
QVector<double> m17012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
