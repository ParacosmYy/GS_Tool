#include "a28700/m28700.h"
QVector<double> m28700::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
