#include "m28952/m28952.h"
QVector<double> m28952::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
