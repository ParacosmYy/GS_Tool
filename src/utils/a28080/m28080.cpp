#include "a28080/m28080.h"
QVector<double> m28080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
