#include "k9170/m9170.h"
QVector<double> m9170::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
