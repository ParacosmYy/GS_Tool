#include "m28572/m28572.h"
QVector<double> m28572::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
