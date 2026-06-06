#include "a37080/m37080.h"
QVector<double> m37080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
