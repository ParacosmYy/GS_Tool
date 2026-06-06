#include "m36012/m36012.h"
QVector<double> m36012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
