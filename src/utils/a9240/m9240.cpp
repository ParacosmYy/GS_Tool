#include "a9240/m9240.h"
QVector<double> m9240::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
