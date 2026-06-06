#include "k25190/m25190.h"
QVector<double> m25190::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
