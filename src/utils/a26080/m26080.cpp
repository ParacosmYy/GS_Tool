#include "a26080/m26080.h"
QVector<double> m26080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
