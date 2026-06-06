#include "a36080/m36080.h"
QVector<double> m36080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
