#include "a28520/m28520.h"
QVector<double> m28520::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
