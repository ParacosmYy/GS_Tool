#include "k28710/m28710.h"
QVector<double> m28710::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
