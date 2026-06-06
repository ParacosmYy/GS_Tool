#include "k18830/m18830.h"
QVector<double> m18830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
