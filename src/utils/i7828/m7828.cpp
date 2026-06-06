#include "i7828/m7828.h"
QVector<double> m7828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
