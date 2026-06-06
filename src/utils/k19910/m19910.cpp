#include "k19910/m19910.h"
QVector<double> m19910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
