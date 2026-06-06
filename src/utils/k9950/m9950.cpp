#include "k9950/m9950.h"
QVector<double> m9950::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
