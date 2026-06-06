#include "k34910/m34910.h"
QVector<double> m34910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
