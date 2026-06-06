#include "k25690/m25690.h"
QVector<double> m25690::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
