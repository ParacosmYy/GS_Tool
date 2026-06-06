#include "k19830/m19830.h"
QVector<double> m19830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
