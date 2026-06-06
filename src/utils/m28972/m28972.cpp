#include "m28972/m28972.h"
QVector<double> m28972::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
