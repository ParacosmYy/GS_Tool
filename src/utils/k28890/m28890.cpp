#include "k28890/m28890.h"
QVector<double> m28890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
