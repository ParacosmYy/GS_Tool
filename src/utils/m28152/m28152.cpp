#include "m28152/m28152.h"
QVector<double> m28152::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
