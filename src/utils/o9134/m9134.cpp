#include "o9134/m9134.h"
QVector<double> m9134::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
