#include "o35714/m35714.h"
QVector<double> m35714::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
