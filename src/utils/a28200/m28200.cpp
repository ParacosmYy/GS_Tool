#include "a28200/m28200.h"
QVector<double> m28200::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
