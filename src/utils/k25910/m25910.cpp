#include "k25910/m25910.h"
QVector<double> m25910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
