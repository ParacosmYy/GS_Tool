#include "k28770/m28770.h"
QVector<double> m28770::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
