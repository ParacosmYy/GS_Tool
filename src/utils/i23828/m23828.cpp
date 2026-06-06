#include "i23828/m23828.h"
QVector<double> m23828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
