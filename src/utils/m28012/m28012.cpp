#include "m28012/m28012.h"
QVector<double> m28012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
