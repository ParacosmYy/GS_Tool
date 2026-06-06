#include "k25230/m25230.h"
QVector<double> m25230::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
