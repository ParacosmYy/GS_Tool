#include "k13910/m13910.h"
QVector<double> m13910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
