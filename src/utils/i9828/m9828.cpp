#include "i9828/m9828.h"
QVector<double> m9828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
