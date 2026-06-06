#include "i8828/m8828.h"
QVector<double> m8828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
