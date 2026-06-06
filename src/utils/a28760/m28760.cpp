#include "a28760/m28760.h"
QVector<double> m28760::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
