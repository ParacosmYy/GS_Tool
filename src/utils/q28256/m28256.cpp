#include "q28256/m28256.h"
QVector<double> m28256::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
