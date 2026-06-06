#include "k28390/m28390.h"
QVector<double> m28390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
