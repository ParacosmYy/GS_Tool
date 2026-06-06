#include "g28426/m28426.h"
QVector<double> m28426::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
