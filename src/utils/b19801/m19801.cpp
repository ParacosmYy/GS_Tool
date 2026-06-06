#include "b19801/m19801.h"
QVector<double> m19801::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
