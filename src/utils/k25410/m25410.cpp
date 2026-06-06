#include "k25410/m25410.h"
QVector<double> m25410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
