#include "a9580/m9580.h"
QVector<double> m9580::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
