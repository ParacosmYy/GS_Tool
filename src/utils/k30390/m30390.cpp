#include "k30390/m30390.h"
QVector<double> m30390::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
