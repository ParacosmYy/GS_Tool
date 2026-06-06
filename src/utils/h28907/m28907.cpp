#include "h28907/m28907.h"
QVector<double> m28907::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
