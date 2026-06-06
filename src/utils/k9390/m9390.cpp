#include "k9390/m9390.h"
QVector<double> m9390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
