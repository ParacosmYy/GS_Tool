#include "k28610/m28610.h"
QVector<double> m28610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
