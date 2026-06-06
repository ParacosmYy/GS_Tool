#include "k28430/m28430.h"
QVector<double> m28430::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
