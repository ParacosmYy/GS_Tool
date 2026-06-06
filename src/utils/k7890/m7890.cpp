#include "k7890/m7890.h"
QVector<double> m7890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
