#include "m25512/m25512.h"
QVector<double> m25512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
